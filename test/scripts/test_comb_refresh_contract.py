"""Check generated refresh routing and execute the Verilator zero-time branch."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]


class CombinationalRefreshTest(unittest.TestCase):
    def test_all_native_wrappers_route_refresh_through_xclock(self):
        routes = {
            "python/dut.py": "self.xclock.RefreshComb()",
            "cpp/dut.hpp": "this->xclock.RefreshComb();",
            "java/dut.java": "this.xclock.RefreshComb();",
            "scala/dut.scala": "this.xclock.RefreshComb()",
            "golang/dut.go": "self.Xclock.RefreshComb()",
            "lua/dut.lua": "self.xclock:RefreshComb()",
        }
        for name, route in routes.items():
            with self.subTest(language=name):
                source = (ROOT / "template" / name).read_text()
                self.assertIn(route, source)
                self.assertNotIn("self.dut.RefreshComb()", source)
                self.assertNotIn("this->dut->RefreshComb()", source)
                self.assertNotIn("this.dut.RefreshComb()", source)
                self.assertNotIn("self.Dut.RefreshComb()", source)
                self.assertNotIn("self.dut:RefreshComb()", source)

    @unittest.skipUnless(shutil.which("c++"), "requires a C++ compiler")
    def test_verilator_refresh_does_not_advance_either_counter(self):
        source = (ROOT / "template/lib/dut_base.cpp").read_text()
        start = source.index("int DutVerilatorBase::Step(")
        end = source.index("\n};", start) + 3
        method = source[start:end].replace("{{__TOP_MODULE_NAME__}}", "Probe")
        program = r'''
#include <cassert>
#include <cstdint>
#define likely(value) (value)
struct VerilatedContext {
    uint64_t now = 0;
    void timeInc(uint64_t delta) { now += delta; }
};
struct VProbe {
    VerilatedContext context;
    unsigned evaluations = 0, steps = 0;
    VerilatedContext* contextp() { return &context; }
    void eval() { ++evaluations; }
    void eval_step() { ++steps; }
};
struct DutVerilatorBase {
    uint64_t cycle = 0;
    VProbe model;
    void* top = &model;
    int Step(uint64_t ncycle, bool dump);
};
''' + method + r'''
int main() {
    DutVerilatorBase dut;
    for (unsigned i = 0; i < 3; ++i) assert(dut.Step(1, false) == 0);
    assert(dut.cycle == 0 && dut.model.context.now == 0);
    assert(dut.model.steps == 3 && dut.model.evaluations == 0);
    assert(dut.Step(2, true) == 0);
    assert(dut.cycle == 2 && dut.model.context.now == 2);
    assert(dut.model.evaluations == 2);
    assert(dut.Step(1, false) == 0);
    assert(dut.cycle == 2 && dut.model.context.now == 2);
    assert(dut.model.steps == 4);
}
'''
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path / "probe.cpp").write_text(program)
            subprocess.run([shutil.which("c++"), "-std=c++17", "probe.cpp", "-o", "probe"],
                           cwd=path, check=True, capture_output=True, text=True)
            subprocess.run([str(path / "probe")], cwd=path, check=True)


if __name__ == "__main__":
    unittest.main()
