#!/usr/bin/env python3
"""Verify every `/* Function start: 0xADDR */` annotation names the function that
actually lives at that address.

This exists because I got it wrong 25 times: names like `GetUiFn9DD0` encode only
the low four hex digits, and filling in the high bits by eye produced annotations
pointing at unrelated functions.  A wrong annotation is worse than no annotation --
`make export-asm` then exports the wrong original and `make report` compares against
it, so the similarity number is meaningless without saying so.

    bin/auditAddresses.py            # report mismatches
    bin/auditAddresses.py --fix      # rewrite annotations to the real address
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')
INVENTORY = os.path.join(ROOT, 'src', 'map')
CODE_DIR = os.path.join(ROOT, 'code-full')
SKIP = {
    '__declspec', 'naked', 'void', 'if', 'while', 'for', 'return', 'TODO',
    'switch', 'sizeof'
}
NAME_RE = re.compile(
    r'\b(operator\s+(?:new|delete)|'
    r'[~A-Za-z_][A-Za-z0-9_]*(?:::[~A-Za-z_][A-Za-z0-9_]*)*)\s*\('
)
CALL_RE = re.compile(r'\bCALL\s+0x([0-9A-Fa-f]+)\b', re.IGNORECASE)

# The linker pulled a large static-CRT band between ix objects.  Calls into
# that band and the import-thunk tail are library dependencies, not omissions.
DEVELOPER_CODE_RANGES = (
    (0x00401000, 0x0046DED9),
    (0x00489990, 0x0048C58A),
)

# MSVC emits this scalar deleting destructor for DebugOverlayConsole.  It must
# not be reconstructed by hand (see AGENTS.md's compiler-glue rule).
COMPILER_GENERATED_CALL_TARGETS = {0x0040FF00}


def find_function_name(lines, marker_line, marker_end, expected_name=None):
    candidates = []
    for line_number in range(marker_line, min(marker_line + 12, len(lines))):
        segment = (lines[line_number][marker_end:]
                   if line_number == marker_line else lines[line_number])
        segment = re.sub(r'^[^\S\n]*\*/', ' ', segment)
        segment = re.sub(r'/\*.*?\*/', ' ', segment)
        if (not segment.strip() or
                segment.lstrip().startswith(('/*', '*', '//', '#'))):
            continue
        for match in NAME_RE.finditer(segment):
            name = match.group(1)
            if name in SKIP:
                continue
            if name == expected_name:
                return name
            candidates.append(name)
    return candidates[0] if candidates else None


def load_disassembly_by_address():
    exports = {}
    if not os.path.isdir(CODE_DIR):
        return exports
    for filename in sorted(os.listdir(CODE_DIR)):
        if not filename.endswith('.disassembled.txt'):
            continue
        path = os.path.join(CODE_DIR, filename)
        with open(path, encoding='utf-8', errors='replace') as stream:
            text = stream.read()
        match = re.search(r'^Address:\s*0x([0-9A-Fa-f]+)\s*$', text,
                          re.MULTILINE)
        if match:
            exports[int(match.group(1), 16)] = (path, text)
    return exports


def is_developer_code(address):
    return any(start <= address < end
               for start, end in DEVELOPER_CODE_RANGES)


def main():
    fix = '--fix' in sys.argv
    fl = {}
    with open(INVENTORY) as stream:
        for line in stream:
            fields = line.split(maxsplit=1)
            if len(fields) == 2 and fields[0].startswith('0x'):
                fl[int(fields[0], 16)] = fields[1].strip()
    byname = {}
    for addr, name in fl.items():
        byname.setdefault(name, addr)

    bad = 0
    markers_by_address = {}
    markers_by_name = {}
    for root, _dirs, files in os.walk(SRC):
        for fn in sorted(files):
            if not fn.endswith(('.c', '.cpp')):
                continue
            path = os.path.join(root, fn)
            lines = open(path).read().split('\n')
            changed = False
            for i, line in enumerate(lines):
                m = re.search(r'Function start: 0x([0-9A-Fa-f]+)', line)
                if not m:
                    continue
                addr = int(m.group(1), 16)
                name = find_function_name(lines, i, m.end(), fl.get(addr))
                if name is None:
                    bad += 1
                    rel = os.path.relpath(path, ROOT)
                    print(f'{rel}:{i+1}: cannot find the annotated function')
                    continue
                expected_address = byname.get(name)
                if expected_address is not None and expected_address != addr:
                    bad += 1
                    rel = os.path.relpath(path, ROOT)
                    print(f'{rel}:{i+1}: 0x{addr:08X} labelled {name}, '
                          f'but {name} is at 0x{expected_address:08X}')
                    if fix:
                        lines[i] = (line[:m.start()]
                                    + f'Function start: 0x{expected_address:X}'
                                    + line[m.end():])
                        addr = expected_address
                        changed = True
                elif addr not in fl:
                    bad += 1
                    rel = os.path.relpath(path, ROOT)
                    print(f'{rel}:{i+1}: 0x{addr:08X} {name} is absent from '
                          'src/map')
                elif fl[addr] != name:
                    bad += 1
                    rel = os.path.relpath(path, ROOT)
                    print(f'{rel}:{i+1}: 0x{addr:08X} is {name}, but src/map '
                          f'names it {fl[addr]}')
                location = (path, i + 1, name)
                previous = markers_by_address.setdefault(addr, location)
                if previous != location:
                    bad += 1
                    rel = os.path.relpath(path, ROOT)
                    previous_rel = os.path.relpath(previous[0], ROOT)
                    print(f'{rel}:{i+1}: duplicate marker 0x{addr:08X}; '
                          f'first seen at {previous_rel}:{previous[1]}')
                previous = markers_by_name.setdefault(name, location)
                if previous != location:
                    bad += 1
                    rel = os.path.relpath(path, ROOT)
                    previous_rel = os.path.relpath(previous[0], ROOT)
                    print(f'{rel}:{i+1}: duplicate function name {name}; '
                          f'first seen at {previous_rel}:{previous[1]}')
            if changed:
                open(path, 'w').write('\n'.join(lines))

    for addr, name in sorted(fl.items()):
        if addr not in markers_by_address:
            bad += 1
            print(f'src/map: 0x{addr:08X} {name} has no source marker')

    # Close the direct-call graph over source-written code.  This catches a
    # retail function that is called by a reconstructed function but omitted
    # from both the source and src/map -- the gap that the old 100% marker
    # report could not see.
    exports = load_disassembly_by_address()
    missing_targets = {}
    for caller, (_path, text) in exports.items():
        if caller not in fl:
            continue
        for target_text in CALL_RE.findall(text):
            target = int(target_text, 16)
            if not is_developer_code(target):
                continue
            if target in fl or target in COMPILER_GENERATED_CALL_TARGETS:
                continue
            missing_targets.setdefault(target, set()).add(caller)
    for target, callers in sorted(missing_targets.items()):
        bad += 1
        caller_text = ', '.join(
            f'{fl[caller]} (0x{caller:08X})' for caller in sorted(callers)
        )
        print(f'src/map: missing internal call target 0x{target:08X}; '
              f'called by {caller_text}')

    if bad == 0:
        print('all Function start annotations and internal calls match the inventory')
        return 0
    print(f'\n{bad} mismatch(es)' + (' fixed' if fix else '; rerun with --fix'))
    return 0 if fix else 1


if __name__ == '__main__':
    sys.exit(main())
