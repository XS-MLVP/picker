#!/usr/bin/env python3
"""Exercise an installed Picker wheel, optionally with its released runtime."""

import ast
from importlib.metadata import version
import os
from pathlib import Path
import subprocess
import sys
import tempfile


picker = str(Path(sys.executable).parent / "picker")
expected_glibc = os.environ.get("EXPECTED_GLIBC")
if expected_glibc:
    assert os.confstr("CS_GNU_LIBC_VERSION") == f"glibc {expected_glibc}"

with tempfile.TemporaryDirectory(prefix="picker-installed-") as directory:
    root = Path(directory)
    output = subprocess.check_output([picker, "--version"], cwd=root, text=True)
    assert version("uc-picker") in output, output
    subprocess.run([picker, "export", "--help"], cwd=root, check=True, capture_output=True)
    invalid = subprocess.run([picker, "--not-a-picker-option"], cwd=root, capture_output=True)
    assert invalid.returncode != 0, "the launcher swallowed the native error"

    templates = subprocess.check_output([picker, "--show_default_template_path"], cwd=root, text=True)
    template_path = Path(templates.strip().splitlines()[-1])
    assert template_path.is_relative_to(Path(sys.prefix)), template_path
    assert (template_path / "uvm/xagent.py").is_file()

    # Release checks use the exact published xcomm wheels. PR checks can run
    # before that dependency has been published and cover the standalone CLI.
    if os.environ.get("CHECK_PICKER_RUNTIME") == "1":
        import xspcomm

        signal = xspcomm.XData(8, xspcomm.XData.InOut)
        signal.Set(0xA5)
        assert signal.U() == 0xA5
        subprocess.run(["swig", "-version"], check=True, capture_output=True)
        source = root / "Adder.sv"
        source.write_text("""module Adder(input logic [7:0] a, b, output logic [8:0] sum);
assign sum = {1'b0, a} + {1'b0, b};
endmodule
""")
        subprocess.run([picker, "pack", "--from-rtl", str(source), "--name", "Adder"], cwd=root, check=True)
        package = root / "Adder"
        for name in ["xagent.py", "__init__.py"]:
            ast.parse((package / name).read_text())
        assert (package / "xagent.sv").is_file()
        transactions = list(package.glob("*_trans.sv"))
        assert len(transactions) == 1, transactions
        transaction = transactions[0].read_text()
        assert "sum" in transaction and "[7:0]" in transaction, transaction
        output = root / "dut"
        result = subprocess.run([
            picker, "export", str(source), "--sname", "Adder", "--lang", "python",
            "--tdir", str(output), "--autobuild", "false",
        ], cwd=root, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if result.returncode:
            print(result.stdout)
            result.check_returncode()
        assert (output / "signals.json").is_file()
        ast.parse((output / "python/dut.py").read_text())
        print("Runtime import, SWIG, RTL packing and Python DUT export passed")

print(f"Installed Picker passed on Python {sys.version_info.major}.{sys.version_info.minor}, "
      f"{os.confstr('CS_GNU_LIBC_VERSION')}")
