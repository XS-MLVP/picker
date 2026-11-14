#!/usr/bin/env python3
"""Single configuration artifact and read-only inspection, without a simulator build."""
import json
import os
from pathlib import Path
import subprocess
import shutil
import tempfile
import unittest


class ConfigDatabaseTest(unittest.TestCase):
    def setUp(self):
        self.root = Path(__file__).resolve().parents[2]
        self.picker = os.environ.get("PICKER_BIN", str(self.root / "build/bin/picker"))
        if not Path(self.picker).is_file():
            self.skipTest("built Picker required")
        self.directory = tempfile.TemporaryDirectory(prefix="picker-config-test-")
        self.addCleanup(self.directory.cleanup)
        self.base = Path(self.directory.name)
        self.source = self.base / "top.sv"
        self.source.write_text("module Top(input logic [7:0] a, output logic [7:0] b); assign b=a; endmodule\n")

    def command(self, *args, success=True):
        result = subprocess.run([self.picker, *map(str, args)], capture_output=True, text=True, timeout=60)
        if success:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0)
        return result

    def export(self, name, simulator="verilator", access="dpi", templates=None):
        target = self.base / name
        self.command("export", self.source, "--sname", "Top", "--sdir", templates or self.root / "template",
                     "--tdir", target, "--sim", simulator, "--rw", access, "--autobuild", "false")
        return target / "Top.xscfg"

    def test_obsolete_installed_templates_do_not_create_sidecars(self):
        templates=self.base/"templates"
        shutil.copytree(self.root/"template",templates)
        for name in ("signals.json","native_layouts.json"):
            (templates/"lib"/name).write_text("{{__OBSOLETE_TEMPLATE_VARIABLE__}}")
        database=self.export("upgrade",templates=templates)
        self.assertEqual([p.name for p in database.parent.rglob("*") if p.suffix in (".json",".yaml",".xscfg")],["Top.xscfg"])
        for name in ("signals.json","native_layouts.json"):
            self.assertEqual((templates/"lib"/name).read_text(),"{{__OBSOLETE_TEMPLATE_VARIABLE__}}")

    def test_two_exports_have_distinct_build_identities(self):
        first=self.export("first")
        second=self.export("second")
        a=json.loads(self.command("config",first).stdout)["model_id"]
        b=json.loads(self.command("config",second).stdout)["model_id"]
        self.assertNotEqual(a,b)

    def test_one_configuration_file_and_no_inspection_sidecars(self):
        for simulator, access in (("verilator", "dpi"), ("verilator", "mem_direct"), ("vcs", "dpi")):
            with self.subTest(simulator=simulator, access=access):
                database = self.export(simulator + access, simulator, access)
                configs = [p.name for p in database.parent.rglob("*") if p.suffix in (".json", ".yaml", ".xscfg")]
                self.assertEqual(configs, ["Top.xscfg"])
                before = sorted(str(p) for p in database.parent.rglob("*"))
                info = json.loads(self.command("config", database).stdout)
                self.assertEqual(info["bytes"], database.stat().st_size)
                self.assertEqual(info["format_version"], 1)
                self.assertTrue(info["model_id"])
                ports = json.loads(self.command("config", database, "--section", "metadata", "--name", "ports").stdout)
                self.assertIsInstance(ports, dict)
                self.assertEqual(before, sorted(str(p) for p in database.parent.rglob("*")))

    def test_corrupt_database_and_invalid_inspection_options_fail_cleanly(self):
        database = self.export("bad")
        original = database.read_bytes()
        changed = bytearray(original)
        changed[8] = 99
        database.write_bytes(changed)
        result = self.command("config", database, success=False)
        self.assertEqual(result.stdout, "")
        self.assertIn("unsupported database version", result.stderr)
        database.write_bytes(original[:16])
        self.assertIn("XSCFG", self.command("config", database, success=False).stderr)
        database.write_bytes(original)
        self.command("config", database, "--name", "ports", success=False)
        self.command("config", database, "--section", "unknown", success=False)
        self.command("config", database, "--section", "metadata", "--name", "missing", success=False)


if __name__ == "__main__":
    unittest.main()
