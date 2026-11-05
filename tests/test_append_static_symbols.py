"""Check that private addresses are recovered from the rebuilt artifacts."""

import importlib.util
from pathlib import Path
import struct
import tempfile
from types import SimpleNamespace
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "append_static_symbols", ROOT / "bin" / "appendStaticSymbols.py"
)
statics = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(statics)


def coff_fixture(section_name=".data", addend=0, public_data=False):
    """A public function referencing a private global four bytes into data."""
    symbols = [("_PublicFunction", 0, 1, 2),
               ("_g_private_00499000$S123", 4, 2, 3)]
    if public_data:
        symbols.append(("_PublicData", 0, 2, 2))
    strings = bytearray(b"\0" * 4)
    records = bytearray()
    for name, value, section, storage in symbols:
        name_offset = len(strings)
        strings.extend(name.encode("ascii") + b"\0")
        records.extend(struct.pack("<IIIhHBB", 0, name_offset, value,
                                   section, 0, storage, 0))
    struct.pack_into("<I", strings, 0, len(strings))
    raw_offset = 20 + 2 * 40
    relocation_offset = raw_offset + 12
    symbol_offset = relocation_offset + 10
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbol_offset,
                         len(symbols), 0, 0)
    text = struct.pack("<8sIIIIIIHHI", b".text", 0, 0, 4,
                       raw_offset, relocation_offset, 0, 1, 0, 0)
    data = struct.pack("<8sIIIIIIHHI", section_name.encode(), 0, 0, 8,
                       raw_offset + 4, 0, 0, 0, 0, 0)
    payload = struct.pack("<I", addend & 0xFFFFFFFF) + b"\0" * 8
    relocation = struct.pack("<IIH", 0, 1, 6)
    return header + text + data + payload + relocation + records + strings


class LinkedImage:
    def __init__(self, linked_value, relocated=True):
        self.linked_value = linked_value
        self.relocated = relocated

    def relocations(self):
        return {0x401000} if self.relocated else set()

    def read(self, address, size):
        if address == 0x401000 and size == 4:
            return struct.pack("<I", self.linked_value)
        return None


class PrivateSymbolTest(unittest.TestCase):
    def recover(self, data, image, entries=None):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "probe.obj"
            path.write_bytes(data)
            if entries is None:
                entries = [SimpleNamespace(symbol="_PublicFunction",
                                           object_file="probe.obj", va=0x401000)]
            return statics.recover(path, entries, image)

    def test_address_suffix_is_not_used_as_linked_address(self):
        result = self.recover(coff_fixture(), LinkedImage(0x502004))
        self.assertEqual(result, [("_g_private_00499000", 0x502004)])

    def test_readonly_data_with_negative_addend_and_library_member(self):
        entries = [SimpleNamespace(symbol="_PublicFunction",
                                   object_file="sound:probe.obj", va=0x401000)]
        result = self.recover(coff_fixture(".rdata", addend=-4),
                              LinkedImage(0x502000), entries)
        self.assertEqual(result, [("_g_private_00499000", 0x502004)])

    def test_missing_pe_relocation_is_rejected(self):
        with self.assertRaisesRegex(ValueError, "missing linked HIGHLOW"):
            self.recover(coff_fixture(), LinkedImage(0x502004, relocated=False))

    def test_conflicting_public_and_relocation_addresses_are_rejected(self):
        entries = [SimpleNamespace(symbol="_PublicFunction",
                                   object_file="probe.obj", va=0x401000),
                   SimpleNamespace(symbol="_PublicData",
                                   object_file="probe.obj", va=0x503000)]
        with self.assertRaisesRegex(ValueError, "inconsistent addresses"):
            self.recover(coff_fixture(public_data=True), LinkedImage(0x502004),
                         entries)

    def test_ambiguous_public_anchor_is_rejected(self):
        entry = SimpleNamespace(symbol="_PublicFunction",
                                object_file="probe.obj", va=0x401000)
        with self.assertRaisesRegex(ValueError, "ambiguous public"):
            self.recover(coff_fixture(), LinkedImage(0x502004), [entry, entry])


if __name__ == "__main__":
    unittest.main()
