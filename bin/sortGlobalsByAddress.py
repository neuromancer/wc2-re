#!/usr/bin/env python3
"""Sort global definitions by original address within a definition file.

MSVC emits data in declaration order, so a definition file whose globals ascend
by original address is what lets the rebuilt .data layout line up with the
retail image.  binary-comp's globals audit reports SOURCE_ORDER_DECREASE for
every definition that breaks the order.

Blocks keep whatever comment lines precede them, and a preprocessor conditional
(#if ... #endif) moves as a single unit keyed on the address it declares.
Definitions with no recoverable address -- the WC1-only `/* no-address */`
entries -- stay attached to the block they currently follow.

    bin/sortGlobalsByAddress.py                # check only, report files
    bin/sortGlobalsByAddress.py --write        # rewrite files in place
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_FILES = ['src/globals.c']
ADDRESS = re.compile(r'_(00[0-9a-f]{6})\b')
NO_ADDRESS = '/* no-address */'


def split_blocks(lines):
    """Return (preamble, [(sort_key, block_lines), ...]).

    The preamble is the file comment and its includes; everything after is a
    sequence of definition blocks.
    """
    start = 0
    for i, line in enumerate(lines):
        if line.startswith('#include'):
            start = i + 1
    while start < len(lines) and not lines[start].strip():
        start += 1
    preamble = lines[:start]

    blocks = []
    pending = []
    i = start
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()
        if not stripped or stripped.startswith('/*') or stripped.startswith('*') \
                or stripped.startswith('//'):
            pending.append(line)
            i += 1
            continue
        body = []
        if stripped.startswith('#if'):
            depth = 0
            while i < len(lines):
                body.append(lines[i])
                s = lines[i].strip()
                if s.startswith('#if'):
                    depth += 1
                elif s.startswith('#endif'):
                    depth -= 1
                    if depth == 0:
                        i += 1
                        break
                i += 1
        else:
            depth = 0
            while i < len(lines):
                body.append(lines[i])
                depth += lines[i].count('{') + lines[i].count('(')
                depth -= lines[i].count('}') + lines[i].count(')')
                if depth <= 0 and lines[i].rstrip().endswith(';'):
                    i += 1
                    break
                i += 1
        blocks.append((pending + body, body))
        pending = []
    return preamble, blocks, pending


def block_address(body):
    for line in body:
        if line.strip().startswith('#'):
            continue
        if NO_ADDRESS in line:
            return None
        found = ADDRESS.findall(line)
        if found:
            return int(found[0], 16)
    return None


def sort_file(path, write):
    with open(path) as handle:
        lines = handle.read().split('\n')
    preamble, blocks, trailer = split_blocks(lines)
    if not blocks:
        return False

    keyed = []
    previous = -1
    for order, (full, body) in enumerate(blocks):
        address = block_address(body)
        if address is None:
            address = previous
        else:
            previous = address
        keyed.append((address, order, full))

    ordered = sorted(keyed, key=lambda entry: (entry[0], entry[1]))
    if [entry[1] for entry in ordered] == list(range(len(keyed))):
        return False
    if not write:
        return True

    out = list(preamble)
    for _address, _order, full in ordered:
        out.extend(full)
    out.extend(trailer)
    with open(path, 'w') as handle:
        handle.write('\n'.join(out))
    return True


def main():
    write = '--write' in sys.argv
    targets = [a for a in sys.argv[1:] if not a.startswith('-')] or DEFAULT_FILES
    unsorted_files = []
    for target in targets:
        path = os.path.join(ROOT, target)
        if sort_file(path, write):
            unsorted_files.append(target)
    if unsorted_files and not write:
        for target in unsorted_files:
            print(f'OUT OF ORDER (run with --write to fix): {target}')
        return 1
    if write:
        for target in unsorted_files:
            print(f'sorted {target}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
