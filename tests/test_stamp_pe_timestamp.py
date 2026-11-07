"""Keep timestamp normalization confined to the actual PE timestamp fields."""

import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "stamp_pe_timestamp", ROOT / "bin" / "stampPeTimestamp.py"
)
timestamps = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(timestamps)


def pe_fixture():
    data = bytearray(0x400)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    data[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", data, 0x84, 0x14C, 1, 0x12345678,
                     0, 0, 224, 0)
    optional = 0x98
    struct.pack_into("<H", data, optional, 0x10B)
    struct.pack_into("<I", data, optional + 28, 0x400000)
    struct.pack_into("<I", data, optional + 92, 16)
    struct.pack_into("<II", data, optional + 144, 0x1040, 84)
    struct.pack_into("<8sIIIIIIHHI", data, optional + 224,
                     b".rdata", 0x200, 0x1000, 0x200, 0x200, 0, 0, 0, 0,
                     0x40000040)
    for index, kind in enumerate((4, 3, 2)):
        struct.pack_into("<IIHHIIII", data, 0x240 + index * 28,
                         0, 0x12345678, 0, 0, kind, 16, 0, 0x300)
    data[0x300:0x310] = b"NB10" + struct.pack("<III", 0, 0xDEADBEEF, 22)
    return data


class TimestampTest(unittest.TestCase):
    def stamp(self, data):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "image.exe"
            path.write_bytes(data)
            count = timestamps.stamp(path, 0x32481D45)
            return count, path.read_bytes()

    def test_updates_four_timestamps_and_preserves_every_other_byte(self):
        original = pe_fixture()
        expected = bytearray(original)
        for offset in (0x88, 0x244, 0x260, 0x27C):
            struct.pack_into("<I", expected, offset, 0x32481D45)
        count, result = self.stamp(original)
        self.assertEqual(count, 4)
        self.assertEqual(result, expected)
        self.assertEqual(self.stamp(result)[1], result)

    def test_mismatched_debug_timestamp_does_not_modify_file(self):
        original = pe_fixture()
        struct.pack_into("<I", original, 0x260, 1)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "image.exe"
            path.write_bytes(original)
            with self.assertRaisesRegex(ValueError, "differs from the PE"):
                timestamps.stamp(path, 0x32481D45)
            self.assertEqual(path.read_bytes(), original)

    def test_nonzero_checksum_is_rejected(self):
        original = pe_fixture()
        struct.pack_into("<I", original, 0x98 + 64, 1)
        with self.assertRaisesRegex(ValueError, "zero image checksum"):
            self.stamp(original)

    def test_debug_directory_in_virtual_padding_is_rejected(self):
        original = pe_fixture()
        struct.pack_into("<II", original, 0x98 + 144, 0x11F0, 84)
        with self.assertRaisesRegex(ValueError, "not backed by section bytes"):
            self.stamp(original)


if __name__ == "__main__":
    unittest.main()
