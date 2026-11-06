"""Exercise the real MSPDB41 seed utility, including read-only verification."""

import os
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
DLL = ROOT / "compilers/msvc41/Bin/MSPDB41.DLL"
SIGNATURE = "0x32385F33"


class PdbSeedTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="pdb-seed-test-", dir=ROOT / "out")
        self.addCleanup(self.temp.cleanup)
        self.pdb = Path(self.temp.name) / "seed.pdb"

    def run_tool(self, action, signature=SIGNATURE, age="3", dll=DLL):
        env = os.environ.copy()
        env["WIBO_PATH"] = str(DLL.parent)
        return subprocess.run(
            [str(ROOT / "wibo"), str(ROOT / "out/createPdbSeed.exe"),
             action, str(dll), str(self.pdb), signature, age],
            cwd=ROOT, env=env, text=True, capture_output=True,
        )

    def assert_success(self, result):
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_create_and_read_only_verification(self):
        self.assert_success(self.run_tool("create"))
        original = self.pdb.read_bytes()
        for _ in range(2):
            self.assert_success(self.run_tool("verify"))
        self.assertNotEqual(self.run_tool("verify", signature="0x32385F34").returncode, 0)
        self.assertNotEqual(self.run_tool("verify", age="4").returncode, 0)
        self.assertEqual(self.pdb.read_bytes(), original)
        self.assert_success(self.run_tool("verify"))

    def test_zero_age_and_existing_output(self):
        self.assert_success(self.run_tool("create", age="0"))
        original = self.pdb.read_bytes()
        self.assertNotEqual(self.run_tool("create", age="0").returncode, 0)
        self.assertEqual(self.pdb.read_bytes(), original)
        self.assert_success(self.run_tool("verify", age="0"))

    def test_invalid_identity_does_not_create_output(self):
        for signature, age in [("0", "3"), ("-1", "3"), ("0x100000000", "3"),
                               ("bad", "3"), (SIGNATURE, "-1"), (SIGNATURE, "4097")]:
            with self.subTest(signature=signature, age=age):
                self.assertEqual(self.run_tool("create", signature, age).returncode, 2)
                self.assertFalse(self.pdb.exists())

    def test_missing_inputs(self):
        self.assertNotEqual(self.run_tool("verify").returncode, 0)
        self.assertFalse(self.pdb.exists())
        self.assertNotEqual(self.run_tool("create", dll=self.pdb.with_name("missing.dll")).returncode, 0)
        self.assertFalse(self.pdb.exists())


if __name__ == "__main__":
    unittest.main()
