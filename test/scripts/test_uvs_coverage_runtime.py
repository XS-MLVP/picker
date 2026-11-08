"""Check a coverage-enabled Adder package using a real UVS runtime."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


@unittest.skipUnless(os.environ.get("PICKER_UVS_DUT_DIR"), "requires a compiled UVS Adder package")
class UvsCoverageRuntimeTest(unittest.TestCase):
    """Run DUTs in separate interpreters and merge their run databases with uvcov."""

    @staticmethod
    def _package_files(package):
        return sorted(path.name for path in package.iterdir() if path.name != "__pycache__")

    def test_two_run_databases_merge_into_one_report(self):
        package = Path(os.environ["PICKER_UVS_DUT_DIR"]).resolve()
        program = '''
from Adder import DUTAdder
from pathlib import Path
import sys
dut = DUTAdder()
assert dut.GetSimulator() == "uvs"
database = dut.GetCoveragePath()
assert database == sys.argv[2]
dut.SetCoverage(sys.argv[1])
dut.a.value = int(sys.argv[3])
dut.b.value = 1
dut.cin.value = 0
dut.Step(1)
assert not Path(sys.argv[1]).exists()
assert dut.dut.Finish() == 0
assert dut.GetCoveragePath() == database
assert dut.dut.Finish() == 0
assert (Path(sys.argv[1]) / "data" / "Adder" / "stmt.dat").is_file()
'''
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            database = package / "Adder.cdb"
            runs = [root / "cases" / "case_first.cdb", root / "cases" / "case_second.cdb"]
            before = self._package_files(package)
            for value, run in enumerate(runs):
                result = subprocess.run(
                    [sys.executable, "-c", program, str(run), str(database), str(value)],
                    cwd=package.parent, capture_output=True, text=True, timeout=120,
                )
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(self._package_files(package), before)
            self.assertFalse(list(database.glob("data/*")))

            report = root / "report"
            uvcov = shutil.which("uvcov") or str(Path(os.environ["UVS_HOME"]) / "bin" / "uvcov")
            subprocess.run([uvcov, "-db", str(database), *map(str, runs), "-detail", "-reportdir", str(report)],
                           capture_output=True, text=True, timeout=120, check=True)
            instances = json.loads((report / "data" / "code_coverage.json").read_text())["instances"]
            adder = [instance for instance in instances if instance.get("stmt") and instance["name"] != "Adder_top"]
            self.assertEqual(adder[0]["stmt"]["summary"]["total"]["un"], 0)
            self.assertGreater(adder[0]["stmt"]["summary"]["total"]["cd"], 0)


if __name__ == "__main__":
    unittest.main()
