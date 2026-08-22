#!/usr/bin/env python3
"""Reimplementation progress summary.

Counts functions carrying a `/* Function start: 0x... */` header in src/ and
compares that against the developer-function inventory in src/map.
"""
import os
import re
import sys
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')
MAPFILE = os.path.join(ROOT, 'src', 'map')

HEADER = re.compile(r'/\*\s*Function start:\s*0x([0-9A-Fa-f]+)\b')
TODO = re.compile(r'/\*\s*TODO')

def main():
    if not os.path.isdir(SRC):
        sys.exit('error: src/ not found')

    done = defaultdict(set)
    stubbed = defaultdict(set)
    regions = {}
    per_file = {}
    for dirpath, _dirs, files in os.walk(SRC):
        for name in sorted(files):
            if not name.endswith(('.c', '.cpp')):
                continue
            path = os.path.join(dirpath, name)
            source_region = ('ix' if os.path.relpath(path, SRC).startswith('ix' + os.sep)
                             else 'game')
            with open(path, errors='replace') as fh:
                lines = fh.readlines()
            impl = stub = 0
            for i, line in enumerate(lines):
                m = HEADER.search(line)
                if not m:
                    continue
                addr = int(m.group(1), 16)
                # a header immediately followed by a TODO comment is a stub
                nxt = lines[i + 1] if i + 1 < len(lines) else ''
                regions[addr] = source_region
                if TODO.search(line) or TODO.search(nxt):
                    stubbed[source_region].add(addr)
                    stub += 1
                else:
                    done[source_region].add(addr)
                    impl += 1
            rel = os.path.relpath(path, ROOT)
            if impl or stub:
                per_file[rel] = (impl, stub)

    total = defaultdict(int)
    inventory = set()
    with open(MAPFILE) as fh:
        for line in fh:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            addr = int(line.split()[0], 16)
            inventory.add(addr)
            if addr not in regions:
                sys.exit(f'error: src/map address 0x{addr:08X} has no source')
            total[regions[addr]] += 1

    for source_region in ('game', 'ix'):
        done[source_region].intersection_update(inventory)
        stubbed[source_region].intersection_update(inventory)

    print(f"{'region':<8} {'implemented':>12} "
          f"{'stubbed':>9} {'total':>7}  progress")
    grand_done = grand_total = 0
    for r in ('game', 'ix'):
        d, s, t = len(done[r]), len(stubbed[r]), total[r]
        grand_done += d
        grand_total += t
        pct = (d / t * 100) if t else 0.0
        bar = '#' * int(pct / 2.5)
        print(f"{r:<8} {d:>12} {s:>9} {t:>7}  "
              f"{pct:5.1f}% {bar}")
    pct = ((grand_done / grand_total * 100)
           if grand_total else 0.0)
    print(f"{'TOTAL':<8} {grand_done:>12} {'':>9} "
          f"{grand_total:>7}  {pct:5.1f}%")

    if per_file:
        print('\nper file (implemented / stubbed):')
        for rel in sorted(per_file):
            impl, stub = per_file[rel]
            print(f"  {rel:<44} {impl:>4} / {stub:<4}")


if __name__ == '__main__':
    main()
