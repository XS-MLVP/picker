"""Check a relocated, coverage-enabled Adder package using a real VCS runtime."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


@unittest.skipUnless(os.environ.get("PICKER_VCS_DUT_DIR"), "requires a compiled VCS Adder package")
class VcsCoverageRuntimeTest(unittest.TestCase):
    """Run DUTs in separate interpreters and verify native per-test snapshots."""

    def test_relocated_package_and_two_test_snapshots(self):
        with tempfile.TemporaryDirectory() as directory:
            package = Path(directory) / "Adder"
            shutil.copytree(os.environ["PICKER_VCS_DUT_DIR"], package, symlinks=True)
            program = '''
from Adder import DUTAdder
from pathlib import Path
import sys
dut = DUTAdder()
assert dut.GetSimulator() == "vcs"
database = dut.GetCoveragePath()
assert database == str(Path("Adder/Adder.vdb").resolve())
dut.SetCoverage(sys.argv[1] + ".vdb")
dut.SetWaveform(str(Path(sys.argv[1] + ".fsdb").resolve()))
dut.a.value = 1
dut.b.value = 2
dut.cin.value = 0
dut.Step(1)
assert dut.sum.value == 3
dut.a.value = 3
dut.Step(1)
assert dut.sum.value == 5
assert dut.dut.Finish() == 0
assert dut.GetCoveragePath() == database
assert dut.dut.Finish() == 0
assert Path(sys.argv[1] + ".fsdb").stat().st_size > 0
'''
            for name in ("case_first", "case_second"):
                result = subprocess.run([sys.executable, "-c", program, name], cwd=directory,
                                        capture_output=True, text=True, timeout=120)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertNotIn("Failed to create FSDB", result.stdout + result.stderr)
            for name in ("case_first", "case_second"):
                result = subprocess.run([str(package / "coverage/coverage"), "--database",
                                         str(package / "Adder.vdb"), "--test", name, "--kind", "toggle"],
                                        capture_output=True, text=True, timeout=120, check=True)
                report = json.loads(result.stdout)
                self.assertTrue(report["success"])
                self.assertEqual(report["query"]["tests"], [name])
                self.assertGreater(report["metrics"]["toggle"]["total"], 0)
                self.assertGreater(report["metrics"]["toggle"]["covered"], 0)


if __name__ == "__main__":
    unittest.main()
