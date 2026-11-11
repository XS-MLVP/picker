"""Verify the mem_direct generator follows the Verilator model's C++ contract."""
from pathlib import Path
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]


def render_makefile() -> str:
    template = (ROOT / "template/mem_direct/Makefile").read_text()
    return (template
            .replace("{% if __SIMULATOR__ == \"verilator\" %}", "ifeq (yes,yes)")
            .replace("{% if __SIMULATOR__ == \"gsim\" %}", "ifeq (never,enabled)")
            .replace("{% endif %}", "endif")
            .replace("{{__XSPCOMM_LIB__}}", "/nonexistent"))


class MemDirectTimingTest(unittest.TestCase):
    def compile_flags(self, timing: bool) -> str:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "mem_direct"
            header = root / "build/DPIProbe/VProbe___024root.h"
            source.mkdir()
            header.parent.mkdir(parents=True)
            (source / "Makefile").write_text(render_makefile())
            (source / "gen_addr.cpp").write_text("")
            header.write_text(
                '#include "verilated_timing.h"\n' if timing else "// no timing\n"
            )
            result = subprocess.run(
                ["make", "-n", "TOP_NAME=Probe"], cwd=source,
                capture_output=True, text=True, check=True,
            )
            return next(
                item for item in result.stdout.splitlines()
                if "gen_addr.cpp" in item and "-c" in item
            )

    def test_timing_model_uses_cxx20_coroutines(self):
        flags = self.compile_flags(timing=True)
        self.assertIn("-std=c++20", flags)
        self.assertIn("-fcoroutines", flags)
        self.assertNotIn("-std=c++17", flags)

    def test_plain_model_keeps_cxx17(self):
        flags = self.compile_flags(timing=False)
        self.assertIn("-std=c++17", flags)
        self.assertNotIn("-fcoroutines", flags)


if __name__ == "__main__":
    unittest.main()
