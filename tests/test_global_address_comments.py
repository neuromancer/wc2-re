"""Keep global audit and sorting tools working with address-free names."""

import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]


def load_tool(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / "bin" / (name + ".py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


widths = load_tool("auditGlobalWidths")
sorter = load_tool("sortGlobalsByAddress")


class GlobalAddressCommentTest(unittest.TestCase):
    def test_scalar_widths_use_declaration_addresses(self):
        with tempfile.TemporaryDirectory() as directory:
            header = Path(directory) / "globals.h"
            header.write_text("""
extern short g_nCount; /* 0x00499000 */
extern unsigned char *g_2_pBuffer; /* 0x00499004 */
extern int g_nUnmapped;
extern int g_nArray[3]; /* 0x00499008 */
""")
            with patch.object(widths, "HEADER", str(header)):
                self.assertEqual(widths.declared_scalars(), {
                    0x499000: ("g_nCount", "short", 2),
                    0x499004: ("g_2_pBuffer", "unsigned char*", 4),
                })

    def test_sort_keeps_trailing_annotations_and_multiline_definitions(self):
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "globals.c"
            source.write_text("""#include "game.h"
int g_nLater = 0x00498000; /* 0x00499010 */
short g_3_anEarlier[2] = {
    1, 2
}; /* 0x00499000 */
""")
            self.assertTrue(sorter.sort_file(str(source), False))
            self.assertTrue(sorter.sort_file(str(source), True))
            result = source.read_text()
            self.assertLess(result.index("g_3_anEarlier"), result.index("g_nLater"))
            self.assertIn("}; /* 0x00499000 */", result)
            self.assertFalse(sorter.sort_file(str(source), False))


if __name__ == "__main__":
    unittest.main()
