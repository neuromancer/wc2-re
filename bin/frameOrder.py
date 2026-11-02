#!/usr/bin/env python3
"""Predict the /Od stack-slot order CL 4.1 gives a function's locals.

CL 4.1 hashes each local into one of 16 buckets and hands out frame slots
from the top of the frame (EBP-4 first) in ascending bucket order; locals that
share a bucket go in reverse declaration order (docs/PATTERNS.md).

    bin/frameOrder.py name1 name2 ...   # names in declaration order

Prints each local's bucket and the predicted slot order.  Sizes are not
modelled: a slot is as wide as its local.
"""
import sys


def identifier_hash(name):
    """C1's identifier hash."""
    value = 0
    for character in name.encode():
        value = ((value >> 4) + 4 * value + character) & 0xFFFFFFFF
    return value


def bucket(name):
    value = identifier_hash(name)
    return (value ^ (value >> 16)) & 0xF


def slot_order(names):
    """Names from EBP-4 downward."""
    indexed = list(enumerate(names))
    indexed.sort(key=lambda item: (bucket(item[1]), -item[0]))
    return [name for _index, name in indexed]


def main(argv):
    if not argv:
        print(__doc__.strip())
        return 1
    for name in argv:
        print(f'{name:32} bucket {bucket(name):2}')
    print('slot order (EBP-4 first):', ', '.join(slot_order(argv)))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
