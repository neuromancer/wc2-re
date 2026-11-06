"""Regression checks for the globals manifest consumed by binary-comp."""

import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    "collect_global_definitions", ROOT / "bin" / "collectGlobalDefinitions.py"
)
collector = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(collector)


class GlobalDefinitionsTest(unittest.TestCase):
    def collect(self, source):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "unit.cpp"
            path.write_text(source)
            return collector.definition_block(path)

    def test_late_initializer_replaces_tentative_declaration(self):
        result = self.collect('''
#include "game.h"
static char g_name_00499000[8];
/* Function start: 0x401000 */
void ShowName(void)
{
    static int local_00499010 = 3;
    Print(g_name_00499000);
}
short g_count_00498000 = 2;
static char g_name_00499000[8] = "{name}";
short g_tail_00499008;
''')
        self.assertEqual(result.count("g_name_00499000"), 1)
        self.assertIn('= "{name}"', result)
        self.assertLess(result.index("g_count_00498000"), result.index("g_name_00499000"))
        self.assertIn("g_tail_00499008", result)
        self.assertNotIn("local_00499010", result)
        self.assertNotIn("Print(", result)

    def test_prototypes_are_not_storage_but_function_pointers_are(self):
        result = self.collect('''
char *GetText(int index);
void *LoadResource(const char *path);
extern short g_elsewhere_00498000;
void *(__cdecl *g_allocate_00498004)(unsigned int) = malloc;
void (__cdecl *g_release_00498008)(void *) = free;
''')
        self.assertNotIn("GetText", result)
        self.assertNotIn("LoadResource", result)
        self.assertNotIn("g_elsewhere_00498000", result)
        self.assertIn("g_allocate_00498004", result)
        self.assertIn("g_release_00498008", result)

    def test_nested_reference_conditionals_and_elif(self):
        result = self.collect('''
#ifdef SDL_PORT
int g_value_00498000 = 100;
#else
#if 0
int g_value_00498000 = 200;
#elif !defined(SDL_PORT)
int g_value_00498000 = 7;
#else
int g_value_00498000 = 300;
#endif
#endif
''')
        self.assertIn("g_value_00498000 = 7", result)
        self.assertEqual(result.count("g_value_00498000"), 1)

    def test_duplicate_initialized_definitions_are_an_error(self):
        with self.assertRaisesRegex(ValueError, "multiple active initialized definitions"):
            self.collect("int g_value_00498000 = 1;\nint g_value_00498000 = 2;\n")

    def test_address_comments_survive_collection(self):
        result = self.collect('''
/* 0x00498000 */
const short first = 7;
short second; // 0x00498004
''')
        self.assertIn("0x00498000", result)
        self.assertIn("0x00498004", result)

    def test_real_late_definitions_are_present(self):
        result = collector.definition_block(ROOT / "src" / "campglob.c")
        self.assertIn('= "TEMPGLOB.000"', result)
        self.assertIn("g_aObjectTypeData", result)
        self.assertEqual(result.count("g_szTemporaryCampaignGlobalsFile"), 1)

    def test_active_initializer_macros_survive_collection(self):
        result = self.collect('''
#ifdef SDL_PORT
#define ALLOCATE native_malloc
#else
#define ALLOCATE malloc
#endif
void *(__cdecl *g_allocate_00498004)(unsigned int) = ALLOCATE;
''')
        self.assertIn("#define ALLOCATE malloc", result)
        self.assertNotIn("native_malloc", result)

    def test_unit_manifest_contains_only_units_with_storage(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            populated = root / "populated.c"
            empty = root / "empty.c"
            populated.write_text('short g_value_00498000 = 7;\n')
            empty.write_text('#define PRIVATE_UNIT\nvoid Update(void) {}\n')
            combined = root / "combined.c"
            units = root / "units"
            with patch("sys.argv", ["collectGlobalDefinitions.py",
                                    "--output", str(combined), "--unit-dir", str(units),
                                    str(populated), str(empty)]):
                self.assertEqual(collector.main(), 0)
            paths = (units / "sources.txt").read_text().splitlines()
            self.assertEqual(len(paths), 1)
            self.assertIn("g_value_00498000 = 7", Path(paths[0]).read_text())
            self.assertIn("g_value_00498000 = 7", combined.read_text())


if __name__ == "__main__":
    unittest.main()
