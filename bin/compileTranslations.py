#!/usr/bin/env python3
"""Compile editable UTF-8 catalogs or refresh marked game UI strings.

The runtime file uses WC2's byte encodings, so neither the DOS text formatter
nor its fixed-size fields ever receive UTF-8. No gettext runtime is needed.
GNU gettext is needed only for --update-ui, not normal builds.
"""
import argparse
import ast
from collections import Counter
import json
from pathlib import Path
import re
import struct
import subprocess
import tempfile


FORMAT_TOKEN = r"%[-+ #0]*[0-9]*(?:\.[0-9]+)?(?:hh|ll|[hljztL])?[A-Za-z%]"
KEY_LABEL = r"\([A-Z](?:/[A-Z])?\)|\[[A-Z]\]"
CUTSCENE_TOKENS = {"{last_name}": 0x85, "{callsign}": 0x86,
                   "{campaign_54}": 0x87, "{campaign_55}": 0x88,
                   "{first_name}": 0x89, "{value}": 0x40}
REQUIRED_TOKEN = r"\{(?:last_name|callsign|campaign_54|campaign_55|first_name|value)\}|\$[CNRP]|" + FORMAT_TOKEN + "|" + KEY_LABEL
MESSAGE_ID = re.compile(r"\n*wc2:[0-9a-f]{16}(?: (?:" + REQUIRED_TOKEN + r"))*\n*\Z")


def message_key(source, cutscene=False):
    """Content ID plus format requirements, without the original prose."""
    if MESSAGE_ID.fullmatch(source):
        return source
    if cutscene:
        for token, byte in CUTSCENE_TOKENS.items():
            source = source.replace(token, chr(byte))
        encoded = source.encode("latin1")
    else:
        encoded = source.encode("cp437")
    if b"\0" in encoded or len(encoded) > 32766:
        raise ValueError("Invalid source string")
    value = 14695981039346656037
    for byte in encoded:
        value = ((value ^ byte) * 1099511628211) & 0xffffffffffffffff
    if cutscene:
        for token, byte in CUTSCENE_TOKENS.items():
            source = source.replace(chr(byte), token)
    tokens = re.findall(REQUIRED_TOKEN, source)
    # Gettext checks matching leading/trailing newlines even with opaque IDs.
    leading = source[:len(source) - len(source.lstrip("\n"))]
    trailing = source[len(source.rstrip("\n")):]
    return leading + " ".join([f"wc2:{value:016x}", *tokens]) + trailing


def keyed_po(document):
    def replace(match):
        lines = match.group().splitlines()
        source = ast.literal_eval(lines[0][6:]) + "".join(ast.literal_eval(line) for line in lines[1:])
        return "msgid " + json.dumps(message_key(source)) if source else match.group()
    return re.sub(r'^msgid "[^\n]*"(?:\n"[^\n]*")*', replace, document, flags=re.MULTILINE)


def update_ui_catalog(root):
    sources = sorted(path.relative_to(root).as_posix()
                     for path in (root / "src").rglob("*")
                     if path.suffix in (".c", ".cpp", ".h"))
    with tempfile.TemporaryDirectory() as directory:
        output = Path(directory) / "ui.pot"
        subprocess.run([
            "xgettext", "--language=C++", "--from-code=UTF-8",
            "--keyword", "--keyword=_", "--keyword=N_",
            "--keyword=C_:1c,2", "--keyword=NC_:1c,2",
            "--add-comments=TRANSLATORS:", "--no-wrap", "--sort-by-file",
            "--omit-header", "--output", str(output), *sources,
        ], cwd=root, check=True)
        body = output.read_text(encoding="utf-8")
    header = ('# UI strings extracted from _(), N_(), C_() and NC_().\n'
              'msgid ""\nmsgstr ""\n'
              '"Project-Id-Version: wc2-re\\n"\n'
              '"MIME-Version: 1.0\\n"\n'
              '"Content-Type: text/plain; charset=UTF-8\\n"\n'
              '"Content-Transfer-Encoding: 8bit\\n"\n\n')
    template = root / "translations/templates/ui.pot"
    template.parent.mkdir(parents=True, exist_ok=True)
    template.write_text(keyed_po(header + body), encoding="utf-8")
    catalog = root / "translations/es/ui.po"
    if catalog.exists():
        # Never fuzzily pair changed format strings or keyboard shortcuts.
        subprocess.run(["msgmerge", "--update", "--backup=none",
                        "--no-fuzzy-matching", "--no-wrap", str(catalog), str(template)], check=True)
    print("Updated UI translation template and merged existing translations.")


def compile_artwork(path):
    data = json.loads(path.read_text())
    if data.get("schema") != 1 or not 1 <= len(data["packets"]) <= 256:
        raise ValueError("Invalid artwork schema/count")
    output = bytearray(b"WCA1" + struct.pack("<I", len(data["packets"])))
    seen = set()
    for packet in data["packets"]:
        name, section = packet["file"], packet["section"]
        if not re.fullmatch(r"[A-Z0-9_]+\.[A-Z0-9]{3}", name) or not 0 <= section < 32768 or (name, section) in seen:
            raise ValueError("Invalid or duplicate artwork packet")
        seen.add((name, section))
        source, target = packet["source_size"], packet["target_size"]
        edits = packet["edits"]
        if not 0 < source <= 16 * 1024 * 1024 or not 0 < target <= 16 * 1024 * 1024 or not 0 < len(edits) <= 65536:
            raise ValueError("Invalid artwork size")
        encoded = name.encode("ascii") + b"\0"
        output.extend(struct.pack("<HH5I", len(encoded), section, source, packet["source_crc32"], target, packet["target_crc32"], len(edits)))
        output.extend(encoded)
        position, result_size = 0, source
        for offset, removed, text in edits:
            literal = bytes.fromhex(text)
            if not position <= offset <= source or not 0 <= removed <= source - offset:
                raise ValueError("Overlapping or out-of-bounds artwork edit")
            position = offset + removed
            result_size += len(literal) - removed
            output.extend(struct.pack("<3I", offset, removed, len(literal)))
            output.extend(literal)
        if result_size != target:
            raise ValueError("Artwork result size differs")
    return output


def read_po(path):
    entries = []
    entry = {}
    field = None
    fuzzy = False
    font = None
    capacity = None
    for line in path.read_text(encoding="utf-8").splitlines() + [""]:
        if not line.strip():
            if entry.get("msgid"):
                if "msgstr" not in entry:
                    raise ValueError(f"Missing translation in {path}")
                entries.append((entry, fuzzy, font, capacity))
            entry, field, fuzzy, font, capacity = {}, None, False, None, None
        elif line.startswith("#~"):
            continue
        elif line.startswith("#, "):
            fuzzy |= "fuzzy" in line[3:].split(", ")
        elif line.startswith("#. Original font set: "):
            font = int(line.rsplit(" ", 1)[1])
        elif line.startswith("#. Original field capacity: "):
            capacity = int(line.split(": ", 1)[1].split()[0])
        elif line.startswith("#"):
            continue
        else:
            match = re.fullmatch(r'(msgctxt|msgid|msgstr) (".*")', line)
            if match:
                field = match[1]
                if field in entry:
                    raise ValueError(f"Duplicate PO field in {path}")
                entry[field] = ast.literal_eval(match[2])
            elif line.startswith('"') and field:
                entry[field] += ast.literal_eval(line)
            else:
                raise ValueError(f"Unsupported PO syntax in {path}: {line[:80]}")
    return entries


def build_catalog(directory):
    messages = {}
    skipped = 0
    seen_keys = set()
    for path in sorted(directory.glob("*.po")):
        for entry, fuzzy, font, capacity in read_po(path):
            context = entry.get("msgctxt", "")
            source = message_key(entry["msgid"])
            key = (context.encode("ascii"), source.strip()[:20].encode("ascii"))
            if key in seen_keys:
                raise ValueError(f"Duplicate message key: {context!r}, {entry['msgid']!r}")
            seen_keys.add(key)
            if fuzzy or not entry["msgstr"]:
                skipped += 1
                continue
            translated = entry["msgstr"]
            if path.name == "ui.po":
                # Both printf and the WC2 formatter consume arguments in order.
                # Keep exact directives, including WC2's %X/%Y/%F/%P commands.
                if Counter(re.findall(KEY_LABEL, source)) != Counter(re.findall(KEY_LABEL, translated)):
                    raise ValueError(f"Keyboard shortcut labels differ: {source!r}")
                if re.findall(FORMAT_TOKEN, source) != re.findall(FORMAT_TOKEN, translated):
                    raise ValueError(f"Format directives differ: {source!r}")
                if re.search(r"%[0-9]+\$|%[^A-Za-z%]*\*", translated):
                    raise ValueError(f"Unsupported positional/dynamic format: {source!r}")
            if Counter(re.findall(r"\$[A-Z](?:[0-9])?", source)) != Counter(re.findall(r"\$[A-Z](?:[0-9])?", translated)):
                raise ValueError(f"Campaign placeholders differ: {context}")
            if Counter(re.findall(r"\{[^{}]+\}", source)) != Counter(re.findall(r"\{[^{}]+\}", translated)):
                raise ValueError(f"Cutscene placeholders differ: {context}")
            encoded = translated.encode("cp1252")
            if context.startswith("cutscene/"):
                if any(byte in encoded for byte in (0x40, 0x85, 0x86, 0x87, 0x88, 0x89)):
                    raise ValueError(f"Reserved cutscene character: {context}")
                # The game's expanded dialogue workspace holds 256 bytes.
                # Name fields are 24 bytes, and signed short values need six.
                expanded = len(encoded)
                for token, byte in CUTSCENE_TOKENS.items():
                    marker = token.encode("ascii")
                    expanded += encoded.count(marker) * ((23 if byte in (0x85, 0x86, 0x89) else 6) - len(marker))
                    encoded = encoded.replace(marker, bytes([byte]))
                if expanded > 255:
                    raise ValueError(f"Expanded cutscene text exceeds 255 bytes: {context}")
            fields = (*key, encoded)
            if context.startswith('radio/'):
                # real_vid_transmit loads into 88 bytes, then adds a ship
                # name and expands pilot fields in the 256-byte workspace.
                expanded = len(encoded) + 21
                for marker in re.findall(r'\$[CNRP]', translated):
                    expanded += 23 - len(marker)
                if len(encoded) > 87 or expanded > 255:
                    raise ValueError(f'Radio text exceeds its workspace: {context}')
            if context.startswith('module/'):
                section = int(context.split('/')[3])
                capacity = {1: 29, 2: 59, 3: 18, 4: 39, 5: 39}[section]
            if capacity is not None and len(fields[2]) > capacity:
                raise ValueError(f"Translation exceeds {capacity} encoded bytes: {context}")
            if any(b"\0" in field or len(field) > 32766 for field in fields):
                raise ValueError(f"Invalid catalog string: {context}")
            messages[key] = fields
    if not messages:
        raise ValueError("No translations found")
    glyph_path = directory / "glyphs.json"
    fonts = json.loads(glyph_path.read_text()) if glyph_path.exists() else {"schema": 1, "fonts": []}
    if fonts.get("schema") != 1:
        raise ValueError("Unsupported glyph schema")
    output = bytearray(b"WCT2" + struct.pack("<II", len(messages), len(fonts["fonts"])))
    for key in sorted(messages):
        fields = tuple(field + b"\0" for field in messages[key])
        output.extend(struct.pack("<3H", *(len(field) for field in fields)))
        output.extend(b"".join(fields))
    seen_fonts = set()
    for font in fonts["fonts"]:
        if font["font"] in seen_fonts or not 0 <= font["font"] < 3 or not 1 <= font["height"] <= 32:
            raise ValueError("Invalid font index or height")
        glyphs = list(font["glyphs"])
        seen_fonts.add(font["font"])
        output.extend(struct.pack("<4BIH", font["font"], font["height"], font["ink"],
                                  font["background"], font["source_crc32"], len(glyphs)))
        seen = set()
        for glyph in glyphs:
            character, width = glyph["character"], glyph["width"]
            pixels = bytes.fromhex(glyph["pixels"])
            if character in seen or not 0 <= character <= 255 or not 0 <= width <= 32 or len(pixels) != width * font["height"]:
                raise ValueError("Invalid glyph")
            seen.add(character)
            output.extend(struct.pack("<BB", character, width) + pixels)
    artwork = directory / "artwork.json"
    if artwork.exists():
        output.extend(compile_artwork(artwork))
    return bytes(output), len(messages), skipped


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=Path("translations/es"))
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument("--output", type=Path)
    action.add_argument("--update-ui", action="store_true",
                        help="Extract marked game UI strings and merge the PO catalog")
    args = parser.parse_args()
    try:
        if args.update_ui:
            update_ui_catalog(Path(__file__).resolve().parent.parent)
            return
        output, count, skipped = build_catalog(args.source)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(output)
        print(f"Compiled {count} messages; {skipped} untranslated/review entries fall back to English.")
    except (OSError, ValueError, KeyError, SyntaxError, UnicodeError,
            struct.error, subprocess.CalledProcessError) as error:
        parser.exit(1, f"Translation processing failed: {error}\n")


if __name__ == "__main__":
    main()
