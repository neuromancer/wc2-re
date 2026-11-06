#!/usr/bin/env python3
"""Set the PE and debug-directory link timestamps after LINK 3.10 runs.

The retail image records 1996-09-24 17:41:25 UTC in all four fields. LINK
writes the current time and offers no timestamp override. This changes only
those timestamp dwords; CodeView's separate PDB signature and age are retained.
With --clear-misc-reserved, also zero IMAGE_DEBUG_MISC.Reserved[3], which
LINK leaves uninitialized on some hosts. The retail image has zeroes there.
"""

import argparse
from pathlib import Path
import struct

from binary_comp.core.pe import PEImage


def timestamp_offsets(image):
    data = image.data
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    optional = pe + 24
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    if struct.unpack_from("<HH", data, pe + 4)[0] != 0x14C:
        raise ValueError("expected an i386 image")
    if optional_size < 152 or struct.unpack_from("<H", data, optional)[0] != 0x10B:
        raise ValueError("expected a PE32 optional header with debug directory")
    if struct.unpack_from("<I", data, optional + 64)[0] != 0:
        raise ValueError("expected the retail build's zero image checksum")
    if struct.unpack_from("<I", data, optional + 92)[0] < 7:
        raise ValueError("missing debug-directory slot")
    rva, size = struct.unpack_from("<II", data, optional + 144)
    offsets = [pe + 8]
    if rva == 0 and size == 0:
        return offsets
    if size == 0 or size % 28:
        raise ValueError("invalid debug-directory length")
    address = image.image_base + rva
    section = image.section_for_va(address)
    if section is None or address - section.start + size > section.rawsize:
        raise ValueError("debug directory is not backed by section bytes")
    raw = section.rawptr + address - section.start
    if raw + size > len(data):
        raise ValueError("truncated debug directory")
    link_time = struct.unpack_from("<I", data, pe + 8)[0]
    for index in range(0, size, 28):
        offset = raw + index + 4
        if struct.unpack_from("<I", data, offset)[0] != link_time:
            raise ValueError("debug timestamp differs from the PE link time")
        offsets.append(offset)
    return offsets


def misc_reserved_offsets(image, timestamps):
    offsets = []
    for timestamp in timestamps[1:]:
        kind, size, _, raw = struct.unpack_from("<IIII", image.data, timestamp + 8)
        if kind != 4:  # IMAGE_DEBUG_TYPE_MISC
            continue
        if size < 12 or size % 4 or raw + size > len(image.data):
            raise ValueError("invalid miscellaneous debug record length")
        data_type, length, unicode = struct.unpack_from("<IIB", image.data, raw)
        if data_type != 1 or length != size or unicode not in (0, 1):
            raise ValueError("expected an IMAGE_DEBUG_MISC_EXENAME record")
        offsets.append(raw + 9)
    return offsets


def stamp(path, timestamp, clear_misc_reserved=False):
    image = PEImage(str(path))
    offsets = timestamp_offsets(image)
    reserved = misc_reserved_offsets(image, offsets) if clear_misc_reserved else []
    data = bytearray(image.data)
    for offset in offsets:
        struct.pack_into("<I", data, offset, timestamp)
    for offset in reserved:
        data[offset:offset + 3] = b"\0\0\0"
    Path(path).write_bytes(data)
    return len(offsets)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    parser.add_argument("timestamp", type=lambda value: int(value, 0))
    parser.add_argument("--clear-misc-reserved", action="store_true",
                        help="zero the reserved bytes of miscellaneous debug records")
    args = parser.parse_args()
    if not 0 <= args.timestamp <= 0xFFFFFFFF:
        parser.error("timestamp must fit an unsigned dword")
    count = stamp(args.image, args.timestamp, args.clear_misc_reserved)
    print(f"Set {count} link timestamps to 0x{args.timestamp:08X}")
    if args.clear_misc_reserved:
        print("Normalized IMAGE_DEBUG_MISC reserved bytes")


if __name__ == "__main__":
    main()
