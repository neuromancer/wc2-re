#!/usr/bin/env python3
"""Build one parser input from globals distributed across owner units."""

from __future__ import annotations

import argparse
import ast
import re
from pathlib import Path


FUNCTION_MARKER = "/* Function start:"
# Definitions start after the unit's own top-level include: the game core
# includes wc1.h, the ix library includes ix.h.
WC1_INCLUDE = '#include "wc1.h"'
IX_INCLUDE = '#include "ix.h"'
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


FALSE_CONDITIONS = ("#if 0", "#ifdef WC1_SDL", "#if defined(WC1_SDL)")
TRUE_CONDITIONS = ("#ifndef WC1_SDL", "#if !defined(WC1_SDL)")


def resolve_conditionals(text: str) -> str:
    """Drop the branches the retail build never compiles.

    The definition files keep the WC1 shape of a global beside the WC2 one
    under `#if 0` or `#ifdef WC1_SDL`.  The parser has no preprocessor, so a
    conditional pair would otherwise declare the same global twice and the
    audit would score the dead branch as a second, overlapping definition.
    """
    out: list[str] = []
    # (emitting, seen_taken_branch) for each open conditional
    stack: list[tuple[bool, bool]] = []
    for line in text.split("\n"):
        stripped = line.strip()
        if stripped.startswith("#if"):
            if stripped.startswith(FALSE_CONDITIONS):
                stack.append((False, False))
                continue
            if stripped.startswith(TRUE_CONDITIONS):
                stack.append((True, True))
                continue
            stack.append((True, True))
            out.append(line)
            continue
        if stripped.startswith("#else") and stack:
            emitting, taken = stack[-1]
            if taken and emitting:
                out.append(line)
            stack[-1] = (not taken, True)
            continue
        if stripped.startswith("#endif") and stack:
            emitting, taken = stack.pop()
            if emitting and taken:
                out.append(line)
            continue
        if all(emitting for emitting, _ in stack):
            out.append(line)
    return "\n".join(out)


def definition_block(path: Path) -> str:
    text = path.read_text(encoding="utf-8")
    if path.name == "globals.c":
        return text

    include_at = text.find(WC1_INCLUDE)
    if include_at < 0:
        include_at = text.find(IX_INCLUDE)
    function_at = text.find(FUNCTION_MARKER)
    if include_at < 0:
        raise ValueError(f"cannot locate global-definition block in {path}")
    if function_at < 0:
        function_at = len(text)
    elif function_at <= include_at:
        raise ValueError(f"cannot locate global-definition block in {path}")
    declarations_at = text.find("\n", include_at) + 1
    return text[declarations_at:function_at].strip() + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--constants-header", action="append", default=[], type=Path)
    parser.add_argument("sources", nargs="+", type=Path)
    args = parser.parse_args()
    constants = header_constants(args.constants_header)

    parts = [
        "/* Generated by bin/collectGlobalDefinitions.py; do not edit. */\n",
        f"{WC1_INCLUDE}\n",
    ]
    for source in args.sources:
        parts.append(f'\n#line 1 "{source.as_posix()}"\n')
        parts.append(
            expand_array_dimensions(
                resolve_conditionals(definition_block(source)), constants
            )
        )
    output = "".join(parts)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_text(encoding="utf-8") != output:
        args.output.write_text(output, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
