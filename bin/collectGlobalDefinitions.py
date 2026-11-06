#!/usr/bin/env python3
"""Build one parser input from globals distributed across owner units."""

from __future__ import annotations

import argparse
import ast
import re
from pathlib import Path

from binary_comp.analyzers.globals import (
    declaration_has_storage,
    declaration_value_and_declarator,
    declarator_identifier_node,
    is_global_declaration_node,
    leading_comments_before,
    trailing_comments_after,
)
from binary_comp.source.cpp import make_cpp_parser, node_text, sanitize_source, walk
from binary_comp.source.preprocessor import (
    PreprocessorConfig,
    blank_inactive_conditionals,
)

INCLUDE = '#include "game.h"'
BLOCK_COMMENT_RE = re.compile(r"/\*.*?\*/", re.DOTALL)
LINE_COMMENT_RE = re.compile(r"//[^\r\n]*")
DEFINE_RE = re.compile(
    r"^[ \t]*#define[ \t]+([A-Za-z_]\w*)[ \t]+([^\r\n]+)",
    re.MULTILINE,
)
ENUM_RE = re.compile(r"\benum(?:\s+[A-Za-z_]\w*)?\s*\{(.*?)\}\s*;", re.DOTALL)
INTEGER_SUFFIX_RE = re.compile(r"(?<=[0-9A-Fa-f])[uUlL]+\b")


def evaluate_integer(expression: str, constants: dict[str, int]) -> int:
    expression = INTEGER_SUFFIX_RE.sub("", expression.strip())
    node = ast.parse(expression, mode="eval")

    def visit(current: ast.AST) -> int:
        if isinstance(current, ast.Expression):
            return visit(current.body)
        if isinstance(current, ast.Constant) and type(current.value) is int:
            return current.value
        if isinstance(current, ast.Name) and current.id in constants:
            return constants[current.id]
        if isinstance(current, ast.UnaryOp):
            value = visit(current.operand)
            if isinstance(current.op, ast.UAdd):
                return value
            if isinstance(current.op, ast.USub):
                return -value
            if isinstance(current.op, ast.Invert):
                return ~value
        if isinstance(current, ast.BinOp):
            left = visit(current.left)
            right = visit(current.right)
            if isinstance(current.op, ast.Add):
                return left + right
            if isinstance(current.op, ast.Sub):
                return left - right
            if isinstance(current.op, ast.Mult):
                return left * right
            if isinstance(current.op, (ast.Div, ast.FloorDiv)):
                quotient = abs(left) // abs(right)
                return -quotient if (left < 0) != (right < 0) else quotient
            if isinstance(current.op, ast.Mod):
                quotient = abs(left) // abs(right)
                if (left < 0) != (right < 0):
                    quotient = -quotient
                return left - quotient * right
            if isinstance(current.op, ast.LShift):
                return left << right
            if isinstance(current.op, ast.RShift):
                return left >> right
            if isinstance(current.op, ast.BitOr):
                return left | right
            if isinstance(current.op, ast.BitAnd):
                return left & right
            if isinstance(current.op, ast.BitXor):
                return left ^ right
        raise ValueError(expression)

    return visit(node)


def header_constants(paths: list[Path]) -> dict[str, int]:
    constants: dict[str, int] = {}
    pending: list[tuple[str, str]] = []

    for path in paths:
        text = BLOCK_COMMENT_RE.sub("", path.read_text(encoding="utf-8"))
        text = LINE_COMMENT_RE.sub("", text)
        pending.extend(DEFINE_RE.findall(text))
        for enum_body in ENUM_RE.findall(text):
            previous_name: str | None = None
            for item in enum_body.split(","):
                item = item.strip()
                if not item:
                    continue
                if "=" in item:
                    name, expression = item.split("=", 1)
                    name = name.strip()
                    expression = expression.strip()
                else:
                    name = item
                    expression = "0" if previous_name is None else f"{previous_name} + 1"
                if re.fullmatch(r"[A-Za-z_]\w*", name):
                    pending.append((name, expression))
                    previous_name = name

    while pending:
        unresolved: list[tuple[str, str]] = []
        changed = False
        for name, expression in pending:
            try:
                constants[name] = evaluate_integer(expression, constants)
                changed = True
            except (SyntaxError, ValueError, ZeroDivisionError):
                unresolved.append((name, expression))
        if not changed:
            break
        pending = unresolved
    return constants


def expand_array_dimensions(text: str, constants: dict[str, int]) -> str:
    expanded: list[str] = []
    index = 0
    while index < len(text):
        if text.startswith("//", index):
            end = text.find("\n", index)
            if end < 0:
                expanded.append(text[index:])
                break
            expanded.append(text[index:end])
            index = end
            continue
        if text.startswith("/*", index):
            end = text.find("*/", index + 2)
            if end < 0:
                expanded.append(text[index:])
                break
            end += 2
            expanded.append(text[index:end])
            index = end
            continue
        if text[index] in {'"', "'"}:
            quote = text[index]
            end = index + 1
            while end < len(text):
                if text[end] == "\\":
                    end += 2
                    continue
                end += 1
                if text[end - 1] == quote:
                    break
            expanded.append(text[index:end])
            index = end
            continue
        if text[index] == "[":
            end = text.find("]", index + 1)
            if end >= 0:
                expression = text[index + 1:end]
                try:
                    value = evaluate_integer(expression, constants)
                except (SyntaxError, ValueError, ZeroDivisionError):
                    pass
                else:
                    expanded.append(f"[{value}]")
                    index = end + 1
                    continue
        expanded.append(text[index])
        index += 1
    return "".join(expanded)


def resolve_conditionals(text: str) -> str:
    """Use the same reference-build conditional filtering as binary-comp."""
    return blank_inactive_conditionals(
        text.encode("utf-8"),
        PreprocessorConfig(enabled=True, undefined=frozenset({"SDL_PORT"})),
    ).decode("utf-8")


def definition_block(path: Path) -> str:
    """Collect file-scope storage, including definitions after functions.

    A tentative declaration before the functions can have its initializer at
    the end of the unit. Keep that definition once, at its actual position;
    the early declaration neither initializes nor orders the object's data.
    """
    source = resolve_conditionals(path.read_text(encoding="utf-8")).encode("utf-8")
    tree = make_cpp_parser().parse(sanitize_source(source))
    definitions = {}
    context = []
    for node in walk(tree.root_node):
        if node.type == "preproc_def" and is_global_declaration_node(node):
            context.append(node)
        if node.type != "declaration" or not is_global_declaration_node(node):
            continue
        initializer, declarator = declaration_value_and_declarator(node)
        if declaration_has_storage(source, node, "extern") and initializer is None:
            continue
        name_node = declarator_identifier_node(declarator)
        if name_node is None:
            continue
        # The first operator binding the name distinguishes a function that
        # returns a pointer from a variable holding a pointer to a function.
        binding = name_node.parent
        while binding is not None and binding.type == "parenthesized_declarator":
            binding = binding.parent
        if binding is not None and binding.type == "function_declarator":
            continue
        name = node_text(source, name_node)
        previous = definitions.get(name)
        if previous is not None:
            previous_initializer, _ = declaration_value_and_declarator(previous)
            if initializer is None:
                continue
            if previous_initializer is not None:
                raise ValueError(f"multiple active initialized definitions of {name} in {path}")
        definitions[name] = node

    if not definitions:
        return "\n"
    parts = []
    for node in sorted([*context, *definitions.values()], key=lambda item: item.start_byte):
        parts.extend((
            leading_comments_before(source, node.start_byte),
            node_text(source, node) + trailing_comments_after(source, node.end_byte),
        ))
    return "\n".join(part for part in parts if part) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--unit-dir", type=Path)
    parser.add_argument("--constants-header", action="append", default=[], type=Path)
    parser.add_argument("sources", nargs="+", type=Path)
    args = parser.parse_args()
    constants = header_constants(args.constants_header)

    preamble = [
        "/* Generated by bin/collectGlobalDefinitions.py; do not edit. */\n",
        f"{INCLUDE}\n",
    ]
    parts = list(preamble)
    unit_paths = []
    for source in args.sources:
        definitions = expand_array_dimensions(definition_block(source), constants)
        unit = f'\n#line 1 "{source.as_posix()}"\n' + definitions
        parts.append(unit)
        if args.unit_dir is not None and definitions.strip():
            unit_path = args.unit_dir / source.as_posix().replace("/", "_")
            write_if_changed(unit_path, "".join(preamble) + unit)
            unit_paths.append(unit_path.as_posix())
    output = "".join(parts)

    write_if_changed(args.output, output)
    if args.unit_dir is not None:
        write_if_changed(args.unit_dir / "sources.txt", "\n".join(unit_paths) + "\n")
    return 0


def write_if_changed(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if not path.exists() or path.read_text(encoding="utf-8") != text:
        path.write_text(text, encoding="utf-8")


if __name__ == "__main__":
    raise SystemExit(main())
