#!/usr/bin/env python3
"""Generic native RTL type/storage alignment; no product-specific designs."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
import zipfile

RTL = r'''
typedef struct packed { logic [7:0] tag; logic [23:0] data; } packet_t;
typedef struct { logic [7:0] tag; logic [64:0] wide; packet_t packet; } record_t;
typedef struct { record_t children [3:2]; logic signed [12:8] signed_value; } outer_t;
module Child #(parameter W=13)(input logic clk, input logic [63:0] din, output logic [63:0] dout);
    packet_t packet /* verilator public_flat_rw */;
    packet_t packets [-1:0] /* verilator public_flat_rw */;
    logic [1:0][31:0] lanes /* verilator public_flat_rw */;
    logic [64:0] memory [3:2][-1:1] /* verilator public_flat_rw */;
    outer_t records [4:3] /* verilator public_flat_rw */;
    logic [W-1:0] parameterized /* verilator public_flat_rw */;
    always_ff @(posedge clk) begin
        packet <= din[31:0];
        packets[-1] <= din[31:0]; packets[0] <= din[63:32];
        lanes <= din;
        parameterized <= W'(din);
        for (int i=2; i<=3; i++)
            for (int j=-1; j<=1; j++) memory[i][j] <= {1'b1, din} ^ 65'(i*4+j);
        for (int i=3; i<=4; i++) begin
            records[i].signed_value <= 5'(i);
            for (int j=2; j<=3; j++) begin
                records[i].children[j].tag <= 8'(i*4+j);
                records[i].children[j].wide <= {1'b1, din};
                records[i].children[j].packet <= din[31:0];
            end
        end
    end
    assign dout = lanes ^ memory[2][-1][63:0] ^ records[3].children[2].wide[63:0]
                ^ {32'b0,packet} ^ 64'(parameterized) ^ 64'(packets[-1]);
endmodule
module NativeViews(input logic clk, input logic [63:0] din, output logic [63:0] dout);
    logic [63:0] a,b;
    Child #(.W(13)) a_child(.clk(clk),.din(din),.dout(a));
    Child #(.W(19)) b_child(.clk(clk),.din(din),.dout(b));
    assign dout=a^b;
endmodule
'''

CHECK = r'''
from NativeViews import DUTNativeViews, xsp
import sys
dut = DUTNativeViews()
try:
    dut.InitClock("clk")
    dut.din.value = 0x123456789abcdef0
    dut.Step(2)
    root="NativeViews_top.NativeViews.a_child."
    packet=dut.GetInternalSignalView(root+"packet").Struct()
    assert packet.Data("tag").value == 0x9a
    packet.Data("tag").value = 0x55
    assert dut.GetInternalSignal(root+"packet").value == 0x55bcdef0
    assert dut.GetInternalSignalView(root+"packet.tag").Data().CSelf() == packet.Data("tag").CSelf()
    packets=dut.GetInternalSignalView(root+"packets").Array()
    assert packets.bounds == ((-1,0),)
    packets.Struct(-1).Data("tag").value=0x66
    assert dut.GetInternalSignal(root+"packets",is_array=True)[0].value == 0x66bcdef0
    selected_packet=dut.GetInternalSignalView(root+"packets[-1]").Struct()
    assert selected_packet.Data("tag").CSelf()==packets.Struct(-1).Data("tag").CSelf()
    lanes=dut.GetInternalSignalView(root+"lanes").Array()
    assert lanes.bounds == ((1,0),)
    lanes.Data(0).value=0x11223344
    assert dut.GetInternalSignal(root+"lanes").value == 0x1234567811223344
    memory=dut.GetInternalSignalView(root+"memory").Array()
    assert memory.bounds == ((3,2),(-1,1))
    memory.Array(3).Data(1).value=(1<<64)|0x55
    assert dut.GetInternalSignal(root+"memory",is_array=True)[1][2].value == (1<<64)|0x55
    assert dut.GetInternalSignalView(root+"memory[3][1]").Data().CSelf()==memory.Array(3).Data(1).CSelf()
    records=dut.GetInternalSignalView(root+"records").Array()
    assert records.bounds == ((4,3),)
    first=records.Struct(4).Array("children").Struct(3)
    second=records.Struct(3).Array("children").Struct(2)
    old=second.Data("wide").value
    first.Data("wide").value=(1<<64)|0x1234
    assert first.Data("tag").value == 19
    assert second.Data("wide").value == old
    first.Struct("packet").Data("data").value=0x765432
    assert first.Struct("packet").Data("tag").value == 0x9a
    selected=dut.GetInternalSignalView(root+"records[4].children[3].wide").Data()
    assert selected.CSelf() == first.Data("wide").CSelf()
    assert selected.value == (1<<64)|0x1234
    small=dut.GetInternalSignalView(root+"records[4].signed_value")
    assert small.IsSigned()
    small.Data().value=31
    assert small.Data().S() == -1
    assert dut.GetInternalSignalView(root+"parameterized").Data().W() == 13
    assert dut.GetInternalSignalView(root.replace("a_child","b_child")+"parameterized").Data().W() == 19
    assert not dut.GetInternalSignalView(root+"missing").HasValue()
    try:
        memory.Array(4)
        raise AssertionError("out-of-range RTL index accepted")
    except IndexError:
        pass
    try:
        dut.GetInternalSignalView(root+"packet",use_vpi=True)
        raise AssertionError("VPI was not enabled")
    except RuntimeError as e:
        assert "--vpi" in str(e)
    print("native views PASS")
finally:
    dut.Finish()
'''

CPP_CHECK = r'''
#include "UT_NativeViews.hpp"
#include <cassert>
int main() {
    UTNativeViews dut;
    const std::string root="NativeViews_top.NativeViews.a_child.";
    auto packet=dut.GetInternalSignalView(root+"packet").Struct();
    packet->PackedData()->Set(0xabcdef12);
    packet->Data("tag")->Set(0x34);
    assert(packet->PackedData()->U()==0x34cdef12);
    auto records=dut.GetInternalSignalView(root+"records").Array();
    auto record=records->Struct(4)->Array("children")->Struct(3);
    record->Data("wide")->Set("0x10000000012345678");
    assert(record->Data("wide")->String()=="010000000012345678");
    assert(dut.GetInternalSignalView(root+"records[4].children[3].wide").Data()==record->Data("wide"));
    auto memory=dut.GetInternalSignalView(root+"memory").Array();
    assert(memory->Left()==3 && memory->Right()==2);
    memory->Array(3)->Data(-1)->Set(42);
    assert(dut.GetInternalSignal(root+"memory",true)->Array(1)->Data(0)->U()==42);
    dut.Finish();
}
'''

JAVA_CHECK = r'''
package com.ut;
import com.ut.NativeViews.*;
import com.xspcomm.*;
public class example {
    public static void main(String[] args) {
        UT_NativeViews dut=new UT_NativeViews();
        String root="NativeViews_top.NativeViews.a_child.";
        XStruct packet=dut.GetInternalSignalView(root+"packet").Struct();
        packet.PackedData().Set("0xabcdef12");
        packet.Data("tag").Set(0x34);
        assert packet.PackedData().U().longValue()==0x34cdef12;
        XArray records=dut.GetInternalSignalView(root+"records").Array();
        XStruct record=records.Struct(4).Array("children").Struct(3);
        record.Data("wide").Set("0x10000000012345678");
        assert record.Data("wide").String().equals("010000000012345678");
        XArray memory=dut.GetInternalSignalView(root+"memory").Array();
        assert memory.Left()==3 && memory.Right()==2;
        memory.Array(3).Data(-1).Set(42);
        assert dut.GetInternalSignal(root+"memory",true).Array(1).Data(0).U().longValue()==42;
        dut.Finish();
    }
}
'''

GO_CHECK = r'''
package main
import ut "UT_NativeViews"
func main() {
    dut:=ut.NewUT_NativeViews()
    root:="NativeViews_top.NativeViews.a_child."
    packet:=dut.GetInternalSignalView(root+"packet").Struct()
    packet.PackedData().SetGo("0xabcdef12")
    packet.Data("tag").SetGo(0x34)
    if packet.PackedData().U64()!=0x34cdef12 { panic("packed slice") }
    record:=dut.GetInternalSignalView(root+"records").Array().Struct(4).Array("children").Struct(3)
    record.Data("wide").SetGo("0x10000000012345678")
    if record.Data("wide").String()!="010000000012345678" { panic("unpacked field") }
    memory:=dut.GetInternalSignalView(root+"memory").Array()
    if memory.Left()!=3 || memory.Right()!=2 { panic("bounds") }
    memory.Array(3).Data(-1).SetGo(42)
    if dut.GetInternalSignalArray(root+"memory").Array(1).Data(0).U64()!=42 { panic("storage coordinates") }
    dut.Finish()
}
'''

LUA_CHECK = r'''
package.path="UT_NativeViews/?.lua;"..package.path
local DUT=require("UT_NativeViews")
local dut=DUT:new()
local root="NativeViews_top.NativeViews.a_child."
local packet=dut:GetInternalSignalView(root.."packet"):Struct()
packet:PackedData():Set("0xabcdef12")
packet:Data("tag"):Set(0x34)
assert(packet:PackedData():U()==0x34cdef12)
local record=dut:GetInternalSignalView(root.."records"):Array():Struct(4):Array("children"):Struct(3)
record:Data("wide"):Set("0x10000000012345678")
assert(record:Data("wide"):String()=="010000000012345678")
local memory=dut:GetInternalSignalView(root.."memory"):Array()
assert(memory:Left()==3 and memory:Right()==2)
memory:Array(3):Data(-1):Set(42)
assert(memory:Array(3):Data(-1):U()==42)
dut:Finish()
'''


class NativeViewsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root=Path(__file__).resolve().parents[2]
        cls.picker=os.environ.get("PICKER_BIN",str(cls.root/"build/bin/picker"))
        if not Path(cls.picker).is_file() or not shutil.which("verilator"):
            raise unittest.SkipTest("built Picker and Verilator required")

    def run_command(self, command, cwd=None, env=None):
        result=subprocess.run(command,cwd=cwd,env=env,capture_output=True,text=True,timeout=600)
        self.assertEqual(result.returncode,0,f"{command}\n{result.stdout[-5000:]}\n{result.stderr[-8000:]}")
        return result.stdout

    def exercise(self, trace, language="python", rebuild=False):
        with tempfile.TemporaryDirectory(prefix="picker-native-view-") as directory:
            base=Path(directory)
            source=base/"native.sv"; source.write_text(RTL)
            output=base/"NativeViews"
            command=[self.picker,"export",str(source),"--sname","NativeViews","--sdir",str(self.root/"template"),
                     "--tdir",str(output),"--lang",language,"--rw","mem_direct","--sim","verilator","--autobuild","false"]
            if trace: command += ["-w","test.vcd"]
            if trace is True: command += ["-V","--trace-structs"]
            if rebuild: command += ["--verbose"]
            self.run_command(command)
            database=output/"NativeViews.xscfg"
            names=json.loads(self.run_command([self.picker,"config",str(database),"--section","candidates"]))
            types=json.loads(self.run_command([self.picker,"config",str(database),"--section","types"]))
            # Two instances reference the same complex type. Elements are not
            # expanded into metadata, even when they contain nested structs.
            self.assertEqual(names["NativeViews_top.NativeViews.a_child.records"],
                             names["NativeViews_top.NativeViews.b_child.records"])
            self.assertFalse(any("records[" in name for name in names))
            self.assertLess(len(types),25)
            if language != "python":
                extension, check = {"cpp":("cpp",CPP_CHECK), "java":("java",JAVA_CHECK),
                                    "golang":("go",GO_CHECK), "lua":("lua",LUA_CHECK)}[language]
                (output/language/("example."+extension)).write_text(check)
            self.run_command(["make","-C",str(output),"NPROC=4"])
            if rebuild:
                final=output/"UT_NativeViews/NativeViews.xscfg"
                first=final.read_bytes()
                self.run_command(["make","-C",str(output),"NPROC=4"])
                self.assertEqual(first,final.read_bytes())
                self.assertEqual(json.loads(self.run_command([self.picker,"config",str(final),"--section","candidates"])),{})
            # One delivered database; no parallel YAML/JSON configuration or
            # index/cache sidecars. JVM packages contain the database in the JAR.
            self.assertFalse(list(output.rglob("*_offset.yaml")))
            self.assertFalse(list(output.rglob("signals.json")))
            self.assertFalse(list(output.rglob("native_layouts.json")))
            if language != "java": self.assertEqual(len(list(output.rglob("*.xscfg"))),1)
            else:
                self.assertEqual(list(output.rglob("*.xscfg")),[])
                with zipfile.ZipFile(output/"UT_NativeViews-java.jar") as jar:
                    self.assertEqual([n for n in jar.namelist() if n.endswith(".xscfg")],["NativeViews.xscfg"])
            if language == "python":
                check=CHECK.replace("from NativeViews import", "from UT_NativeViews import") if rebuild else CHECK
                self.run_command([sys.executable,"-c",check],cwd=output if rebuild else base)
            elif language == "cpp":
                self.run_command([str(output/"UTNativeViews_example")],cwd=output)
            elif language == "java":
                library=self.run_command([self.picker,"--show_xcom_lib_location_java"]).strip().splitlines()[-1]
                self.run_command(["java","-ea","-cp",os.pathsep.join([library,str(output/"UT_NativeViews-java.jar")]),"com.ut.example"],cwd=output)
            elif language == "golang":
                library=self.run_command([self.picker,"--show_xcom_lib_location_golang"]).strip().splitlines()[-1]
                self.run_command(["go","run","example.go"],cwd=output,
                    env={**os.environ,"GO111MODULE":"off","GOPATH":os.pathsep.join([str(output/"golang"),library])})
            else:
                self.run_command(["lua","example.lua"],cwd=output)

    def test_without_trace_or_vpi(self):
        self.exercise(False)

    def test_incremental_build_keeps_one_database(self):
        self.exercise("flat",rebuild=True)

    def test_trace_structs_without_vpi(self):
        self.exercise(True)

    def test_flat_trace_preserves_semantic_layout(self):
        self.exercise("flat")

    def test_cpp(self):
        self.exercise(False,"cpp")

    def test_java(self):
        if not shutil.which("javac"): self.skipTest("Java toolchain unavailable")
        self.exercise(False,"java")

    def test_golang(self):
        if not os.environ.get("NATIVE_EXTRA_LANGUAGES"): self.skipTest("Set NATIVE_EXTRA_LANGUAGES=1")
        self.exercise(False,"golang")

    def test_lua(self):
        if not os.environ.get("NATIVE_EXTRA_LANGUAGES"): self.skipTest("Set NATIVE_EXTRA_LANGUAGES=1")
        self.exercise(False,"lua")


if __name__ == "__main__":
    unittest.main()
