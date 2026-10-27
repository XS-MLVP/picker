"""Verify simulator metadata using actual picker template generation without a compiler."""

import ast
import os
from pathlib import Path
import subprocess
import tempfile
import unittest


class SimulatorMetadataTest(unittest.TestCase):
    """Generated DUT class metadata must describe the selected backend."""

    def test_python_simulator_metadata(self):
        """Render VCS and Verilator wrappers and check the exported class attribute."""
        root = Path(__file__).resolve().parents[2]
        picker = os.environ.get("PICKER_BIN", str(root / "build/bin/picker"))
        for simulator in ("vcs", "verilator"):
            with self.subTest(simulator=simulator), tempfile.TemporaryDirectory() as directory:
                result = subprocess.run([
                    picker, "export", str(root / "example/Adder/Adder.v"),
                    "--sim", simulator, "--rw", "dpi", "--sname", "Adder",
                    "--tdir", directory + "/", "--sdir", str(root / "template"),
                    "--autobuild", "false",
                ], capture_output=True, text=True, timeout=60)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                wrapper = Path(directory) / "Adder/python/dut.py"
                tree = ast.parse(wrapper.read_text())
                dut_class = next(node for node in tree.body
                                 if isinstance(node, ast.ClassDef) and node.name == "DUTAdder")
                attribute = next(node for node in dut_class.body
                                 if isinstance(node, ast.Assign)
                                 and any(isinstance(target, ast.Name) and target.id == "SIMULATOR"
                                         for target in node.targets))
                self.assertEqual(ast.literal_eval(attribute.value), simulator)
                methods = {node.name for node in dut_class.body if isinstance(node, ast.FunctionDef)}
                self.assertIn("GetSimulator", methods)
                if simulator == "vcs":
                    self.assertIn("GetCoveragePath", methods)
                    self.assertTrue((Path(directory) / "Adder/coverage/vcs_uncover.cpp").is_file())


if __name__ == "__main__":
    unittest.main()
