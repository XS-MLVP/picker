#!/usr/bin/env python3
"""Build regressions for review findings: wrapper names and C++ packed inout."""
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest


NAMES = ("vpi_backend", "_vpi_backend", "vpiBackend", "_finished", "_picker_view_value",
         "GetInternalSignalView", "signal_array_map", "signal_map", "xcfg", "SIMULATOR",
         "pin_vpi_backend", "pin__vpi_backend", "pin_vpiBackend", "pin__finished",
         "pin__picker_views", "p_view", "p_view_")
RTL = """typedef struct packed {logic [7:0] tag;} packet_t;
module ReviewNames(
    input packet_t p,
    input logic [7:0] _picker_views [0:1],
    %s,
    output logic [7:0] seen
);
assign seen = p.tag ^ _picker_views[0] ^ _picker_views[1] ^ %s;
endmodule
""" % (",\n    ".join("input logic [7:0] " + name for name in NAMES), " ^ ".join(NAMES))

PYTHON = r'''
from ReviewNames import DUTReviewNames, xsp
dut = DUTReviewNames()
dut.Step(1)
names = NAMES
for index, name in enumerate(names): dut.xport[name].value = index + 1
dut.xport["p"].value = 42
dut["_picker_views"].Data(0).value = 9
dut["_picker_views"].Data(1).value = 17
dut.Step(1)
expected = 42 ^ 9 ^ 17
for index, name in enumerate(names):
    assert dut.xport[name].value == index + 1
    expected ^= index + 1
assert dut.seen.value == expected
assert isinstance(dut.pin_vpi_backend, xsp.XData)
assert dut.pin_vpi_backend.CSelf() == dut.xport["pin_vpi_backend"].CSelf()
assert dut.pin_vpi_backend_2.CSelf() == dut.xport["vpi_backend"].CSelf()
assert dut.GetInternalSignalView("vpi_backend", use_vpi=True).Data().U() == 1
assert dut.GetInternalSignal("_vpi_backend", use_vpi=True).U() == 2
dut.Finish()
'''.replace("NAMES", repr(NAMES))

CPP = r'''
#include "UT_ReviewNames.hpp"
#include <cassert>
int main() {
    UTReviewNames dut;
    dut.Step(1);
    const char *names[] = {NAMES};
    unsigned expected = 42 ^ 9 ^ 17, index = 0;
    for (const auto name : names) { dut.xport[name].Set(++index); expected ^= index; }
    dut.p.Set(42);
    dut.pin__picker_views_2->Data(0)->Set(9);
    dut.pin__picker_views_2->Data(1)->Set(17);
    dut.Step(1);
    assert(dut.seen.U() == expected);
    assert(dut.GetInternalSignalView("vpi_backend", true).Data()->U() == 1);
    dut.Finish();
}
'''.replace("NAMES", ", ".join('"' + n + '"' for n in NAMES))

JAVA = r'''
package com.ut;
import com.ut.ReviewNames.*;
public class example {
    public static void main(String[] args) {
        UT_ReviewNames dut = new UT_ReviewNames();
        dut.Step(1);
        String[] names = {NAMES};
        int expected = 42 ^ 9 ^ 17;
        for (int i=0; i<names.length; ++i) {dut.xport.Get(names[i]).Set(i+1); expected ^= i+1;}
        dut.p.Set(42);
        dut.pin__picker_views_2.Data(0).Set(9);
        dut.pin__picker_views_2.Data(1).Set(17);
        dut.Step(1);
        assert dut.seen.U().intValue() == expected;
        assert dut.GetInternalSignalView("vpi_backend", true).Data().U().intValue() == 1;
        dut.Finish();
    }
}
'''.replace("NAMES", ", ".join('"' + n + '"' for n in NAMES))

GO = r'''
package main
import dutpkg "UT_ReviewNames"
func main() {
    dut := dutpkg.NewUT_ReviewNames()
    dut.Step(1)
    names := []string{NAMES}
    expected := uint64(42 ^ 9 ^ 17)
    for i, name := range names {dut.Xport.Get(name).SetGo(i+1); expected ^= uint64(i+1)}
    dut.P.SetGo(42)
    dut.Pin__picker_views_2.Data(0).SetGo(9)
    dut.Pin__picker_views_2.Data(1).SetGo(17)
    dut.Step(1)
    if dut.Seen.U64() != expected {panic("renamed leaves/views collided")}
    if dut.GetInternalSignalView("vpi_backend", true).Data().U64() != 1 {panic("VPI backend collided")}
    dut.Finish()
}
'''.replace("NAMES", ", ".join('"' + n + '"' for n in NAMES))

LUA = r'''
local DUT = require("UT_ReviewNames")
local dut = DUT:new()
dut:Step(1)
local names = {NAMES}
local expected = 42 ~ 9 ~ 17
for i, name in ipairs(names) do dut.xport:Get(name):Set(i); expected = expected ~ i end
dut.p:Set(42)
dut.pin__picker_views_2:Data(0):Set(9)
dut.pin__picker_views_2:Data(1):Set(17)
dut:Step(1)
assert(dut.seen:U() == expected)
assert(dut:GetInternalSignalView("vpi_backend", true):Data():U() == 1)
assert(dut._finished == false)
dut:Finish()
assert(dut._finished == true)
'''.replace("NAMES", ", ".join('"' + n + '"' for n in NAMES))


class CodegenReviewTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).resolve().parents[2]
        cls.picker = os.environ.get("PICKER_BIN", str(cls.root/"build/bin/picker"))
        if not Path(cls.picker).is_file() or not shutil.which("verilator"):
            raise unittest.SkipTest("built Picker and Verilator required")

    def command(self, command, cwd=None, env=None):
        result = subprocess.run(command, cwd=cwd, env=env, text=True, capture_output=True, timeout=600)
        self.assertEqual(result.returncode, 0, str(command) + "\n" + result.stdout[-5000:] + result.stderr[-8000:])
        return result.stdout

    def test_reserved_members_across_bindings(self):
        languages = ("python", "cpp", "java", "golang", "lua") if os.environ.get("REVIEW_EXTRA_LANGUAGES") else ("python", "cpp", "java")
        for language in languages:
            for access in ("dpi", "mem_direct"):
                with self.subTest(language=language, access=access), tempfile.TemporaryDirectory(prefix="picker-review-") as directory:
                    base = Path(directory); source = base/"review.sv"; source.write_text(RTL)
                    output = base/"ReviewNames"
                    self.command([self.picker,"export",str(source),"--sname","ReviewNames","--sdir",str(self.root/"template"),
                        "--tdir",str(output),"--lang",language,"--rw",access,"--vpi","--autobuild","false"])
                    if language != "python":
                        ext, check = {"cpp":("cpp",CPP),"java":("java",JAVA),"golang":("go",GO),"lua":("lua",LUA)}[language]
                        (output/language/("example."+ext)).write_text(check)
                    self.command(["make","-C",str(output),"NPROC=4"])
                    if language == "python": self.command([sys.executable,"-c",PYTHON],cwd=base)
                    elif language == "cpp": self.command([str(output/"UTReviewNames_example")],cwd=output)
                    elif language == "java":
                        jar = self.command([self.picker,"--show_xcom_lib_location_java"]).strip().splitlines()[-1]
                        self.command(["java","-ea","-cp",os.pathsep.join((jar,str(output/"UT_ReviewNames-java.jar"))),"com.ut.example"],cwd=output)
                    elif language == "golang":
                        library = self.command([self.picker,"--show_xcom_lib_location_golang"]).strip().splitlines()[-1]
                        self.command(["go","run","example.go"],cwd=output,env={**os.environ,"GO111MODULE":"off","GOPATH":os.pathsep.join((str(output/"golang"),library))})
                    else: self.command(["lua","example.lua"],cwd=output)

    def test_cpp_packed_and_vector_inout(self):
        rtl = '''typedef struct packed {logic [3:0] tag; logic valid;} packet_t;
module ReviewInout(input logic [4:0] drive, inout packet_t bus, inout logic [4:0] plain, output logic [4:0] seen);
assign bus=packet_t'(drive);
assign plain=drive;
assign seen=bus ^ plain;
endmodule\n'''
        check = r'''
#include "UT_ReviewInout.hpp"
#include <cassert>
int main() {
    UTReviewInout dut;
    assert(dut.bus.IsBiIO() && dut.plain.IsBiIO());
    assert(dut.bus_view->PackedData().get() == &dut.bus);
    // A real DUT-driven inout is observable. An undriven inout can be folded
    // to zero by Verilator; that is not a test of external tri-state drive.
    dut.drive.Set(19);
    dut.Step(1);
    assert(dut.bus.U() == 19 && dut.plain.U() == 19);
    assert(dut.bus_view->Data("tag")->U() == 9);
    assert(dut.bus_view->Data("valid")->U() == 1);
    dut.drive.Set(6);
    dut.Step(1);
    assert(dut.bus.U() == 6 && dut.plain.U() == 6);
    assert(dut.bus_view->Data("tag")->U() == 3);
    dut.Finish();
}
'''
        for access in ("dpi", "mem_direct"):
            with self.subTest(access=access), tempfile.TemporaryDirectory(prefix="picker-inout-") as directory:
                base=Path(directory); source=base/"inout.sv"; source.write_text(rtl); output=base/"ReviewInout"
                self.command([self.picker,"export",str(source),"--sname","ReviewInout","--sdir",str(self.root/"template"),
                    "--tdir",str(output),"--lang","cpp","--rw",access,"--autobuild","false"])
                self.assertIn("IOType::InOut",(output/"cpp/dut.cpp").read_text())
                self.assertNotIn("IOType::Inout",(output/"cpp/dut.cpp").read_text())
                (output/"cpp/example.cpp").write_text(check)
                self.command(["make","-C",str(output),"NPROC=4"])
                self.command([str(output/"UTReviewInout_example")],cwd=output)


if __name__ == "__main__":
    unittest.main()
