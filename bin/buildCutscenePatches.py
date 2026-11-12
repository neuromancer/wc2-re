#!/usr/bin/env python3
"""Generate src/sdl/cutscene_patches.h: frame-rate bounds for cutscene shots
whose motion is counted in drawn frames.

    python3 bin/buildCutscenePatches.py --original data/dos/GAMEDAT

DOS and Kilrathi Saga decode these INCIDENT.S00 and INCIDENT.S02 sections
byte for byte alike; either GAMEDAT works, provided it holds Special
Operations 2. Nothing is downloaded or written outside the header.

The cutscene VM keeps two clocks. Sprite, plane and sequence waits count 60 Hz
ticks, but a script that moves an object once per loop iteration moves it once
per presented frame. Both releases clear the frame delay whenever a campaign
script starts (InitializeCutsceneRuntimeResources on Win32; the DOS overlay
does the same before allocating its sprite arrays), so those loops ran as fast
as the machine drew. The scripts' own geometry implies the authors' machines
drew these shots at about ten to twelve iterations a second; the Saga presents
silent cutscenes at 70 Hz. Three shots then come apart:

* showfly (sections 98-102): a five-second shot of the player's fighter. The
  ship planes start at x = -80 and advance two pixels every third iteration;
  the hull artwork ends at x = 0. At 70 Hz the plane reaches about +38, so the
  hull stops short of the right edge and the cockpit floats in space.
* whoa (section 113): Spirit's fighter drifts left one pixel per iteration for
  100 ticks, then the explosion starts at the fixed x = 180, 20 pixels in front
  of the fighter's starting point. At 70 Hz the fighter is near x = 84 when the
  fireball appears, about 100 pixels away from it.
* __running__ (Special Operations 2, section 11): a five-second Morningstar
  flyby over four 320x128 star-field tiles at x 0/-320, y 0/-128. The field
  scrolls (+2, +1) per iteration; past x = 320 or y = 128 the tiles no longer
  cover the viewport and the uncovered pixels keep stale contents. At 70 Hz
  the field reaches about (354, 177).

Opcode 0x8d sets the delay between presented frames in ticks; both releases
implement it, and no shipped film uses it. Each fix sets the delay where the
sequence starts and clears it before the sequence yields, so nothing outside
the shot changes. showfly and __running__ present twice per iteration, so 3
ticks give ten iterations a second, the rate the Saga itself uses while
cutscene speech plays (SetCinematicFrameTiming(20)); the fighter ends near
x = -46 and the star field near (100, 50). Special Operations 1's finale
(INCIDENT.S01 section 23, unpatched) confirms the rate: its warp burst shrinks
from scale 100 by 6 per iteration under a 100-tick timer, reaching zero just as
the timer ends only at six ticks per iteration. whoa presents
once per iteration; 5 ticks give 20 iterations in its 100-tick timer, which is
exactly where the script places the explosion.

The generator inserts the opcodes at structural anchors, relocates every IFF
length, script offset and branch they cross, re-decodes all scripts, and emits
only the changed bytes. The runtime checks length and CRC32 before and after.
"""
import argparse
import difflib
from pathlib import Path
import struct
import zlib


# filename, section, source CRC32, sequence, ticks per presented frame, reason.
FIXES = [
    ("INCIDENT.S00", 98, 0x36178346, "showfly", 3,
     "Rapier flight shot: keep the panning hull under the cockpit."),
    ("INCIDENT.S00", 99, 0x2cb3aaa7, "showfly", 3,
     "Sabre flight shot, including the final mission's K'Tithrak Mang scene:\n"
     "keep the panning hull under the cockpit."),
    ("INCIDENT.S00", 100, 0xe355e256, "showfly", 3,
     "Epee flight shot: keep the panning hull under the cockpit."),
    ("INCIDENT.S00", 101, 0xd24b0a7f, "showfly", 3,
     "Broadsword flight shot: keep the panning hull under the cockpit."),
    ("INCIDENT.S00", 102, 0xf77f82ad, "showfly", 3,
     "Ferret flight shot: keep the panning hull under the cockpit."),
    ("INCIDENT.S00", 113, 0x4e6f07e8, "whoa", 5,
     "Series 5 mission 3, Enemy Base: start the explosion on Spirit's fighter."),
    ("INCIDENT.S02", 11, 0xda91ec7a, "__running__", 3,
     "Special Operations 2 Morningstar flyby: keep the scrolling star field\n"
     "under the whole viewport."),
]

FRAME_DELAY = 0x8d
PUSH_BYTE = 0x24
JUMP = 0x0e
YIELD = 0x23
BRANCHES = {0x0e, 0x0f} | set(range(0x10, 0x17))

# Operand bytes after each opcode, from RunCutsceneScript (gameflow.c).
# Variable forms are handled in instruction_length().
OPERAND_BYTES = {
    **{op: 2 for op in BRANCHES},
    0x1f: 1, 0x20: 1, 0x21: 1, 0x24: 1, 0x25: 2, 0x27: 2, 0x29: 2,
    0x3d: 1, 0x4f: 1, 0x57: 1, 0x63: 1, 0x6c: 1, 0x6d: 1, 0x6e: 1, 0x6f: 2,
    0x70: 1, 0x71: 1, 0x74: 1, 0x75: 1, 0x77: 1, 0x79: 2, 0x7a: 2, 0x7f: 1,
    0x82: 2, 0x83: 3, 0x84: 1, 0x85: 1, 0x87: 3, 0x88: 1, 0x89: 1, 0x8a: 1,
    0x8f: 2, 0x90: 1, 0x91: 1, 0x92: 2, 0x93: 1, 0x94: 1, 0x95: 1, 0x97: 1,
    0x9e: 1, 0xa2: 1, 0xa5: 1, 0xa6: 3, 0xa8: 2, 0xac: 2, 0xad: 1, 0xae: 1,
    0xaf: 1, 0xb1: 2, 0xb2: 1, 0xb3: 2, 0xb9: 2,
}
NO_OPERANDS = (set(range(0x00, 0x0e)) | set(range(0x17, 0x1f)) | {0x22, 0x23} |
               set(range(0x2a, 0x3d)) | set(range(0x3e, 0x4f)) |
               {0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x58} |
               set(range(0x59, 0x63)) | set(range(0x64, 0x6c)) |
               {0x7c, 0x7d, 0x7e, 0x80, 0x81, 0x86, 0x8b, 0x8c, 0x8d, 0x8e, 0x96,
                0x98, 0x9a, 0x9b, 0x9c, 0x9d, 0x9f, 0xa1, 0xa3, 0xa4, 0xa7, 0xa9,
                0xaa, 0xab, 0xb0, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8})


def u32(data, offset=0):
    return struct.unpack_from("<I", data, offset)[0]


def decompress_lzw(data):
    """Origin's little-endian, 9-to-12-bit LZW (as in src/sdl/resources.c)."""
    expected = u32(data)
    data = data[4:]
    position = 0
    width = 9
    dictionary = [bytes([i]) for i in range(256)] + [b"", b""]
    previous = b""
    result = bytearray()
    first = True
    while True:
        start = position // 8
        code = (int.from_bytes(data[start:start + 3], "little") >>
                (position % 8)) & ((1 << width) - 1)
        position += width
        if position > len(data) * 8:
            raise ValueError("Truncated LZW section")
        if first and code != 256:
            raise ValueError("Missing LZW clear code")
        first = False
        if code == 256:
            del dictionary[258:]
            width = 9
            previous = b""
            continue
        if code == 257:
            break
        if code < len(dictionary):
            word = dictionary[code]
        elif code == len(dictionary) and previous:
            word = previous + previous[:1]
        else:
            raise ValueError("Invalid LZW code")
        result.extend(word)
        if previous:
            if len(dictionary) >= 4096:
                raise ValueError("LZW dictionary overflow")
            dictionary.append(previous + word[:1])
            if len(dictionary) == 1 << width and width < 12:
                width += 1
        previous = word
    if len(result) != expected:
        raise ValueError("LZW output length mismatch")
    return bytes(result)


def packet_sections(data):
    if len(data) < 8 or u32(data) != len(data):
        raise ValueError("Invalid packet length")
    header_size = u32(data, 4) & 0xffffff
    entries = [u32(data, i) for i in range(4, header_size, 4)] + [len(data)]
    sections = []
    for entry, following in zip(entries, entries[1:]):
        section = data[entry & 0xffffff:following & 0xffffff]
        sections.append(decompress_lzw(section) if entry >> 24 in (1, 0x20) else section)
    return sections


def chunks(film, start, end, path=()):
    """Yield (path, tag, header offset, data offset, size) for every IFF chunk,
    FORMs included; a FORM's data starts at its four-byte type."""
    position = start
    while position < end:
        tag = film[position:position + 4]
        size = struct.unpack_from(">I", film, position + 4)[0]
        data = position + 8
        if data + size > end:
            raise ValueError(f"Chunk {tag!r} at {position:#x} overruns its container")
        yield path, tag, position, data, size
        if tag == b"FORM":
            yield from chunks(film, data + 4, data + size, path + (film[data:data + 4],))
        position = data + ((size + 1) & ~1)
    if position != end:
        raise ValueError(f"Container ending at {end:#x} has trailing bytes")


def script_tables(film):
    """Every script table (OFST inside a FORM SCRP): its kind, symbols, script
    starts and the end of its chunk."""
    tables = []
    symbols = {}
    for path, tag, header, data, size in chunks(film, 0, len(film)):
        if path[-1:] == (b"SCRP",) and tag == b"SYMB":
            symbols[path] = film[data:data + size].split(b"\0")
        if path[-1:] == (b"SCRP",) and tag == b"OFST":
            count = struct.unpack_from("<H", film, data)[0]
            entries = []
            for index in range(count):
                entry = data + 2 + 4 * index
                offset = struct.unpack_from("<i", film, entry)[0]
                entries.append((entry, None if offset == -1 else entry + offset))
            tables.append(dict(kind=path[-2], path=path, data=data, end=data + size,
                               entries=entries))
    for table in tables:
        names = symbols.get(table["path"], [])
        table["names"] = [names[i].decode("ascii") if i < len(names) else str(i)
                          for i in range(len(table["entries"]))]
    return tables


def instruction_length(film, position):
    opcode = film[position]
    if opcode in (0x26, 0x28, 0x99, 0x78):
        cursor = position + (4 if opcode == 0x78 else 1)
        while film[cursor] == 0xff:
            cursor += 1
        return cursor + 1 - position
    if opcode == 0x7b:
        return 3 if 0 <= struct.unpack_from("b", film, position + 1)[0] <= 3 else 2
    if opcode in OPERAND_BYTES:
        return 1 + OPERAND_BYTES[opcode]
    if opcode in NO_OPERANDS:
        return 1
    raise ValueError(f"Illegal cutscene opcode {opcode:#04x} at {position:#x}")


def decode(film, start, end):
    """Linear decode of one script: [(position, opcode, length, branch target)]."""
    result = []
    position = start
    while position < end:
        length = instruction_length(film, position)
        target = None
        if film[position] in BRANCHES:
            operand = position + 1
            target = operand + struct.unpack_from("<h", film, operand)[0]
        result.append((position, film[position], length, target))
        position += length
    if position != end:
        raise ValueError(f"Script at {start:#x} does not end on an instruction boundary")
    return result


def script_bounds(table):
    starts = sorted({start for _, start in table["entries"] if start is not None})
    return {start: (starts[i + 1] if i + 1 < len(starts) else table["end"])
            for i, start in enumerate(starts)}


def check_film(film):
    """Every script decodes exactly and every branch lands on an instruction of
    its own script."""
    for table in script_tables(film):
        for start, end in script_bounds(table).items():
            code = decode(film, start, end)
            boundaries = {position for position, _, _, _ in code}
            for position, _, _, target in code:
                if target is not None and target not in boundaries:
                    raise ValueError(f"Branch at {position:#x} lands inside an instruction")


def insert(film, insertions):
    """Insert bytecode before instructions of one script table, relocating every
    IFF length, script offset and branch displacement that spans an insertion.
    All relocations are computed against the original layout, so the chunk
    padding of the result is checked once, not between insertions."""
    insertions = sorted(insertions)
    points = [position for position, _ in insertions]
    if len(set(points)) != len(points):
        raise ValueError("Two insertions share one position")

    def growth(offset, inclusive):
        return sum(len(payload) for position, payload in insertions
                   if position < offset or (inclusive and position == offset))

    result = bytearray(film)
    table = next(t for t in script_tables(film) if t["data"] < points[0] < t["end"])
    if points[0] <= max(entry for entry, _ in table["entries"]) + 4 or points[-1] >= table["end"]:
        raise ValueError("Insertions must lie inside the scripts of one table")
    for start, end in script_bounds(table).items():
        if start in points:
            raise ValueError("An insertion point starts a script")
        code = decode(film, start, end)
        boundaries = {location for location, _, _, _ in code}
        for position in points:
            if start < position < end and position not in boundaries:
                raise ValueError(f"Insertion at {position:#x} splits an instruction")
        for location, _, _, target in code:
            if target in points:
                raise ValueError(f"Insertion at {target:#x} is a branch target")
            if target is not None:
                operand = location + 1
                moved_operand = operand + growth(operand, True)
                moved_target = target + growth(target, False)
                struct.pack_into("<h", result, operand, moved_target - moved_operand)
    for entry, target in table["entries"]:
        if target is not None:
            struct.pack_into("<i", result, entry, target + growth(target, False) - entry)
    for _, tag, header, data, length in chunks(film, 0, len(film)):
        added = sum(len(payload) for position, payload in insertions
                    if data <= position < data + length)
        if added % 2:
            raise ValueError(f"{tag!r} at {header:#x} would change its IFF padding")
        struct.pack_into(">I", result, header + 4, length + added)
    for position, payload in reversed(insertions):
        result[position:position] = payload
    return bytes(result)


def bound_sequence(film, sequence, delay):
    """Set the frame delay where the sequence resumes and clear it just before
    its final jump back to the yield."""
    table = next(t for t in script_tables(film) if t["kind"] == b"SEQU")
    start = table["entries"][table["names"].index(sequence)][1]
    code = decode(film, start, script_bounds(table)[start])
    first_yield = next(i for i, item in enumerate(code) if item[1] == YIELD)
    resume = code[first_yield + 1][0]
    last = code[-1]
    if last[1] != JUMP or last[3] != code[first_yield][0]:
        raise ValueError(f"{sequence} does not end by jumping back to its yield")
    if any(item[1] == FRAME_DELAY for item in code):
        raise ValueError(f"{sequence} already sets a frame delay")
    film = insert(film, [(resume, bytes([PUSH_BYTE, delay, FRAME_DELAY])),
                         (last[0], bytes([PUSH_BYTE, 0, FRAME_DELAY]))])
    check_film(film)
    return film


def differences(source, target):
    """Changed byte runs, keeping nearby changes together so each relocated
    length or displacement stays readable."""
    edits = []
    matcher = difflib.SequenceMatcher(None, source, target, autojunk=False)
    for tag, i, j, k, l in matcher.get_opcodes():
        if tag == "equal":
            continue
        if edits and i - (edits[-1][0] + edits[-1][1]) <= 8:
            offset, removed, literal = edits.pop()
            literal += source[offset + removed:i] + target[k:l]
            edits.append((offset, j - offset, literal))
        else:
            edits.append((i, j - i, target[k:l]))
    return edits


def emit_patch(lines, filename, section, source, target, comment):
    edits = differences(source, target)
    rebuilt = bytearray()
    position = 0
    for offset, removed, literal in edits:
        rebuilt += source[position:offset] + literal
        position = offset + removed
    rebuilt += source[position:]
    if rebuilt != target:
        raise ValueError(f"{filename} #{section}: edits do not reproduce the patch")
    name = filename.lower().replace(".", "_") + f"_{section}_pacing"
    literals = b"".join(literal for _, _, literal in edits)
    lines += ["/* " + comment.replace("\n", "\n * ") + " */",
              f"static const unsigned char {name}Bytes[] = {{"]
    for i in range(0, len(literals), 12):
        lines.append("    " + ", ".join(f"0x{x:02x}" for x in literals[i:i + 12]) + ",")
    lines += ["};", f"static const SdlDataEdit {name}Edits[] = {{"]
    literal_offset = 0
    for offset, removed, literal in edits:
        lines.append(f"    {{ 0x{offset:04x}, {removed}, {len(literal)}, {literal_offset} }},")
        literal_offset += len(literal)
    lines += ["};", ""]
    return [f'    {{ "{filename}", {section}, {len(source)}, 0x{zlib.crc32(source):08x}U,',
            f"      {len(target)}, 0x{zlib.crc32(target):08x}U,",
            f"      {name}Edits, SDL_arraysize({name}Edits),",
            f"      {name}Bytes, sizeof({name}Bytes) }},"]


def generate(original_root):
    lines = ["/* Generated by bin/buildCutscenePatches.py; see its docstring.",
             " * Each patch sets the cutscene frame delay (opcode 0x8d) for one shot whose",
             " * motion is counted in presented frames, and clears it again before the",
             " * sequence yields. Offsets refer to the ORIGINAL decoded section; the",
             " * runtime verifies its length and CRC32, then the corrected CRC32. */", ""]
    descriptors = []
    sections = {}
    for filename, section, crc, sequence, delay, reason in FIXES:
        if filename not in sections:
            path = original_root / filename
            if not path.is_file():
                raise SystemExit(f"{path} is missing; use a GAMEDAT that includes "
                                 "Special Operations 2, such as data/dos/GAMEDAT")
            sections[filename] = packet_sections(path.read_bytes())
        source = sections[filename][section]
        if zlib.crc32(source) != crc:
            raise ValueError(f"{filename} #{section}: unreviewed revision")
        check_film(source)
        target = bound_sequence(source, sequence, delay)
        comment = (f"{filename}, section {section}, sequence {sequence}: "
                   f"{delay} ticks per frame.\n{reason}")
        descriptors += emit_patch(lines, filename, section, source, target, comment)
    lines += ["static const SdlDataPatch cutscenePacketPatches[] = {",
              *descriptors, "};", ""]
    return "\n".join(lines)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--original", type=Path, required=True,
                        help="DOS or Kilrathi Saga GAMEDAT directory")
    parser.add_argument("--output", type=Path,
                        default=Path(__file__).resolve().parents[1] / "src/sdl/cutscene_patches.h")
    args = parser.parse_args()
    args.output.write_text(generate(args.original), encoding="utf-8")
    print(f"Wrote {len(FIXES)} cutscene pacing patches to {args.output}")
