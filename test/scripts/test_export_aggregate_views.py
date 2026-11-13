#!/usr/bin/env python3
"""Real RTL regression for xcomm-owned recursive packed/unpacked views."""
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import unittest


PYTHON_CHECK = r'''
from AggregatePorts import DUTAggregatePorts, xsp

dut = DUTAggregatePorts()
assert isinstance(dut.packed_in, xsp.XData)
assert isinstance(dut.packed_in_view, xsp.XStruct)
assert dut.packed_in_view.fields == ("tag", "valid")
assert dut.packed_in_view.Array("tag").bounds == ((0, 4),)
dut.packed_in.value = 0
dut.packed_in_view.Data("tag").value = 29
dut.packed_in_view.Data("valid").value = 1
assert dut.packed_in.value == 59
assert dut.packed_in_view.Data("tag").S() == -3
assert dut["packed_in"].CSelf() == dut.packed_in.CSelf()

assert dut.matrix_view.shape == (2, 4)
assert dut.matrix_view[1].bounds == ((-2, 1),)
dut.matrix.value = 0
dut.matrix_view[1][-2].value = 1
dut.matrix_view[0][1].value = 1
assert dut.matrix.value == 0x81
assert dut.matrix_view[1][-2].W() == 0

assert isinstance(dut.record_in, xsp.XStruct)
assert not dut.record_in.IsPacked()
assert tuple(dut.record_in.entries.indices) == (-1, 0)
dut.record_in.enabled.value = 1
first = dut.record_in.entries[-1]
second = dut.record_in.entries[0]
first.header.Data("tag").value = 17
first.header.Data("valid").value = 1
first.flag.value = 1
first.payload[3].value = (1 << 64) + 0x1234
first.payload[2].value = (1 << 63) + 0x5678
second.header.Data("tag").value = 3
second.flag.value = 0
second.payload[3].value = 0x42
assert first.payload[3].CSelf() == dut["record_in.entries[-1].payload[3]"].CSelf()
assert dut["record_in"] is dut.record_in

assert isinstance(dut.packed_records[2], xsp.XStruct)
dut.packed_records[2].Data("tag").value = 5
dut.packed_records[2].Data("valid").value = 1
dut.packed_records[1].PackedData().value = 0x23
count = dut.xport.PortCount()
dut.Step(1)
assert dut.packed_out.value == 59
assert dut.matrix_out.value == 0x81
assert dut.record_out.enabled.value == 1
assert dut.record_out.entries[-1].header.Data("tag").value == 17
assert dut.record_out.entries[-1].payload[3].value == (1 << 64) + 0x1234
assert dut.record_out.entries[-1].payload[2].value == (1 << 63) + 0x5678
assert dut.record_out.entries[0].header.Data("tag").value == 3
assert dut.record_out.entries[0].payload[3].value == 0x42
assert dut.packed_records_out[2].PackedData().value == 11
assert dut.packed_records_out[1].PackedData().value == 0x23
assert count == dut.xport.PortCount() == 26
try:
    dut.record_in.entries[-2]
    raise AssertionError("out-of-range RTL index accepted")
except IndexError:
    pass
dut.Finish()
'''

CPP_CHECK = r'''
#include "UT_AggregatePorts.hpp"
#include <cassert>
int main() {
    UTAggregatePorts dut;
    dut.packed_in.Set(0);
    dut.packed_in_view->Data("tag")->Set(29);
    dut.packed_in_view->Data("valid")->Set(1);
    assert(dut.packed_in.U() == 59);
    dut.matrix.Set(0);
    dut.matrix_view->Array(1)->Data(-2)->Set(1);
    dut.matrix_view->Array(0)->Data(1)->Set(1);
    auto first = dut.record_in->Array("entries")->Struct(-1);
    first->Struct("header")->Data("tag")->Set(17);
    first->Array("payload")->Data(3)->Set("0x10000000000001234");
    dut.record_in->Data("enabled")->Set(1);
    dut.packed_records->Struct(2)->Data("valid")->Set(1);
    const auto count = dut.xport.PortCount();
    dut.Step(1);
    assert(dut.packed_out.U() == 59);
    assert(dut.matrix_out.U() == 0x81);
    auto output = dut.record_out->Array("entries")->Struct(-1);
    assert(output->Struct("header")->Data("tag")->U() == 17);
    XData expected(65, IOType::Input);
    expected.Set("0x10000000000001234");
    assert(output->Array("payload")->Data(3)->Equal(expected));
    assert(dut.packed_records_out->Struct(2)->Data("valid")->U() == 1);
    assert(count == 26 && dut.xport.PortCount() == count);
    dut.Finish();
}
'''

JAVA_CHECK = r'''
package com.ut;
import com.ut.AggregatePorts.*;
import com.xspcomm.*;
public class example {
    public static void main(String[] args) {
        UT_AggregatePorts dut = new UT_AggregatePorts();
        dut.packed_in.Set(0);
        dut.packed_in_view.Data("tag").Set(29);
        dut.packed_in_view.Data("valid").Set(1);
        dut.matrix.Set(0);
        dut.matrix_view.Array(1).Data(-2).Set(1);
        XStruct first = dut.record_in.Array("entries").Struct(-1);
        first.Struct("header").Data("tag").Set(17);
        first.Array("payload").Data(3).Set("0x10000000000001234");
        dut.Step(1);
        assert dut.packed_out.U().longValue() == 59;
        assert dut.matrix_out.U().longValue() == 128;
        XStruct output = dut.record_out.Array("entries").Struct(-1);
        assert output.Struct("header").Data("tag").U().longValue() == 17;
        XData expected = new XData(65, XData.In);
        expected.Set("0x10000000000001234");
        assert output.Array("payload").Data(3).Equal(expected);
        assert dut.xport.PortCount() == 26;
        dut.Finish();
    }
}
'''

GO_CHECK = r'''
package main
import ut "UT_AggregatePorts"
func main() {
    dut := ut.NewUT_AggregatePorts()
    dut.Packed_in.Set(0)
    dut.Packed_in_view.Data("tag").SetGo(29)
    dut.Packed_in_view.Data("valid").SetGo(1)
    dut.Matrix.Set(0)
    dut.Matrix_view.Array(1).Data(-2).SetGo(1)
    first := dut.Record_in.Array("entries").Struct(-1)
    first.Struct("header").Data("tag").SetGo(17)
    first.Array("payload").Data(3).SetGo("0x10000000000001234")
    dut.Step(1)
    if dut.Packed_out.U().Uint64() != 59 { panic("packed struct mismatch") }
    if dut.Matrix_out.U().Uint64() != 128 { panic("packed array mismatch") }
    output := dut.Record_out.Array("entries").Struct(-1)
    if output.Struct("header").Data("tag").U64() != 17 { panic("nested struct mismatch") }
    if output.Array("payload").Data(3).String() != "010000000000001234" { panic("wide field mismatch") }
    if dut.Xport.PortCount() != 26 { panic("views registered extra signals") }
    dut.Finish()
}
'''

LUA_CHECK = r'''
package.path = "UT_AggregatePorts/?.lua;" .. package.path
local DUT = require("UT_AggregatePorts")
local dut = DUT:new()
dut.packed_in:Set(0)
dut.packed_in_view:Data("tag"):Set(29)
dut.packed_in_view:Data("valid"):Set(1)
dut.matrix:Set(0)
dut.matrix_view:Array(1):Data(-2):Set(1)
local first = dut.record_in:Array("entries"):Struct(-1)
first:Struct("header"):Data("tag"):Set(17)
first:Array("payload"):Data(3):Set("0x10000000000001234")
dut:Step(1)
assert(dut.packed_out:U() == 59)
assert(dut.matrix_out:U() == 128)
local output = dut.record_out:Array("entries"):Struct(-1)
assert(output:Struct("header"):Data("tag"):U() == 17)
assert(output:Array("payload"):Data(3):String() == "010000000000001234")
assert(dut.xport:PortCount() == 26)
dut:Finish()
'''

SCALA_CHECK = r'''
package com.ut
import com.ut.AggregatePorts.UT_AggregatePorts
object example {
    def main(args: Array[String]): Unit = {
        val dut = new UT_AggregatePorts()
        dut.packed_in.Set(0)
        dut.packed_in_view.Data("tag").Set(29)
        dut.packed_in_view.Data("valid").Set(1)
        dut.matrix.Set(0)
        dut.matrix_view.Array(1).Data(-2).Set(1)
        val first = dut.record_in.Array("entries").Struct(-1)
        first.Struct("header").Data("tag").Set(17)
        first.Array("payload").Data(3).Set("0x10000000000001234")
        dut.Step(1)
        assert(dut.packed_out.U().longValue() == 59)
        assert(dut.matrix_out.U().longValue() == 128)
        val output = dut.record_out.Array("entries").Struct(-1)
        assert(output.Struct("header").Data("tag").U().longValue() == 17)
        assert(output.Array("payload").Data(3).String() == "010000000000001234")
        assert(dut.xport.PortCount() == 26)
        dut.Finish()
    }
}
'''

VPI_CHECK = r'''
import sys
from AggregatePorts import DUTAggregatePorts, xsp

dut = DUTAggregatePorts()
dut.packed_in.Set(59)
dut.matrix.Set(0x81)
dut.record_in.enabled.Set(1)
dut.packed_records[2].PackedData().Set(0x2f)
dut.Step(1)

if sys.argv[1] == "unpacked-struct":
    cached = dut.GetInternalSignal("packed_in", use_vpi=True)
    if __VPI_STRUCT_SUPPORTED__:
        record = dut.GetInternalSignal("record_in", use_vpi=True)
        assert isinstance(record, xsp.XStruct)
        assert record.Data("enabled").U() == 1
        assert tuple(record.entries.indices) == (-1, 0)
        record.entries[-1].header.Data("tag").Set(17)
        assert dut.GetInternalSignal("record_in.entries[-1].header", use_vpi=True).Data("tag").U() == 17
        dut.Step(1)
        assert dut.record_out.entries[-1].header.Data("tag").U() == 17
    else:
        try:
            dut.GetInternalSignal("record_in", use_vpi=True)
        except RuntimeError as error:
            assert "__VPI_STRUCT_ERROR__" in str(error), str(error)
        else:
            raise AssertionError("unpacked-struct VPI access unexpectedly succeeded")
    # A rejected lookup must not invalidate previously cached, supported views.
    assert dut.GetInternalSignal("packed_in", use_vpi=True).Data("valid").CSelf() == cached.Data("valid").CSelf()
    assert cached.Data("valid").U() == 1
    dut.Finish()
    sys.exit(0)

packed = dut.GetInternalSignal("packed_in", use_vpi=True)
assert isinstance(packed, xsp.XStruct)
assert packed.Data("tag").S() == -3
assert packed.Data("valid").U() == 1
assert isinstance(dut.GetInternalSignal("packed_in.tag", use_vpi=True), xsp.XArray)
qualified = dut.dut.GetVPIScope() + ".packed_in.tag"
assert dut.GetInternalSignal(qualified, use_vpi=True).bounds == ((0, 4),)
held_view = dut.GetInternalSignalView("packed_in")
try:
    held_view.Array()
    raise AssertionError("wrong XView kind was accepted")
except ValueError as error:
    assert "actual=struct, requested=array" in str(error)

matrix = dut.GetInternalSignal("matrix", use_vpi=True)
assert isinstance(matrix, xsp.XArray)
assert matrix.bounds == ((1, 0), (-2, 1))
listed_matrix_name = next(name for name in dut.GetInternalSignalList(use_vpi=True)
                          if name.endswith(".matrix") or name == "matrix")
assert dut.GetInternalSignal(listed_matrix_name, use_vpi=True).bounds == matrix.bounds
assert matrix[1][-2].U() == 1
assert matrix[0][1].U() == 1
matrix[1][-1].Set(1)

records = dut.GetInternalSignal("packed_records", use_vpi=True)
assert isinstance(records, xsp.XArray)
assert records.bounds == ((2, 1),)
records[1].Data("valid").Set(1)

dut.Step(1)
assert dut.packed_out.U() == 59
assert dut.matrix_out.U() == 0xC1
assert dut.packed_records_out[1].PackedData().U() & 1 == 1
held = packed.Data("valid")
dut.Finish()
try:
    held_view.Struct()
    raise AssertionError("closed VPI view converted after Finish")
except RuntimeError:
    pass
try:
    held.U()
    raise AssertionError("VPI view remained usable after its DUT was closed")
except RuntimeError:
    pass
'''

CPP_VPI_CHECK = r'''
#include "UT_AggregatePorts.hpp"
#include <cassert>
#include <stdexcept>
int main(int argc, char **argv) {
    UTAggregatePorts dut;
    dut.packed_in.Set(59);
    dut.matrix.Set(0x81);
    dut.record_in->Data("enabled")->Set(1);
    dut.Step(1);
    if (argc > 1 && std::string(argv[1]) == "unpacked-struct") {
        const auto cached = dut.GetInternalSignalView("packed_in").Struct();
        if (__VPI_STRUCT_SUPPORTED__) {
            std::shared_ptr<XStruct> record = dut.GetInternalSignalView("record_in");
            assert(record->Data("enabled")->U() == 1);
            record->Array("entries")->Struct(-1)->Struct("header")->Data("tag")->Set(17);
            dut.Step(1);
            assert(dut.record_out->Array("entries")->Struct(-1)->Struct("header")->Data("tag")->U() == 17);
        } else {
            bool rejected = false;
            try { (void)dut.GetInternalSignalView("record_in"); }
            catch (const std::runtime_error &error) {
                if (std::string(error.what()).find("__VPI_STRUCT_ERROR__") == std::string::npos) throw;
                rejected = true;
            }
            assert(rejected);
        }
        assert(dut.GetInternalSignalView("packed_in").Struct()->Data("valid") == cached->Data("valid"));
        assert(cached->Data("valid")->U() == 1);
        dut.Finish();
        return 0;
    }
    std::shared_ptr<XStruct> packed = dut.GetInternalSignalView("packed_in");
    const auto held_view = dut.GetInternalSignalView("packed_in");
    bool wrong_kind = false;
    try { (void)held_view.Array(); } catch (const std::invalid_argument &) { wrong_kind = true; }
    assert(wrong_kind);
    assert(packed->Data("tag")->S() == -3);
    assert(packed->Data("valid")->U() == 1);
    std::string matrix_name = "matrix";
    for (const auto &name : dut.GetInternalSignalList("", 99, true)) {
        if (name.size() >= 7 && name.compare(name.size() - 7, 7, ".matrix") == 0) {
            matrix_name = name;
            break;
        }
    }
    std::shared_ptr<XArray> matrix = dut.GetInternalSignalView(matrix_name);
    assert(matrix->Array(1)->Data(-2)->U() == 1);
    matrix->Array(1)->Data(-1)->Set(1);
    std::shared_ptr<XArray> records = dut.GetInternalSignalView("packed_records");
    records->Struct(1)->Data("valid")->Set(1);
    dut.Step(1);
    assert(dut.packed_out.U() == 59);
    assert(dut.matrix_out.U() == 0xc1);
    assert(dut.packed_records_out->Struct(1)->Data("valid")->U() == 1);
    auto held = packed->Data("valid");
    dut.Finish();
    bool expired = false;
    try { held->U(); } catch (const std::runtime_error &) { expired = true; }
    assert(expired);
    expired = false;
    try { (void)held_view.Struct(); } catch (const std::runtime_error &) { expired = true; }
    assert(expired);
    {
        UTAggregatePorts abandoned;
        auto live = abandoned.GetInternalSignalView("packed_in").Struct();
        assert(live != nullptr);
    } // Destructor must close VPI handles before unloading the simulator.
}
'''

JAVA_VPI_CHECK = r'''
package com.ut;
import com.ut.AggregatePorts.*;
import com.xspcomm.*;
public class example {
    public static void main(String[] args) {
        UT_AggregatePorts dut = new UT_AggregatePorts();
        dut.packed_in.Set(59);
        dut.matrix.Set(0x81);
        dut.record_in.Data("enabled").Set(1);
        dut.Step(1);
        if (args.length > 0 && args[0].equals("unpacked-struct")) {
            XStruct cached = dut.GetInternalSignalView("packed_in").Struct();
            if (__VPI_STRUCT_SUPPORTED__) {
                XStruct record = dut.GetInternalSignalView("record_in").Struct();
                assert record.Data("enabled").U().longValue() == 1;
                record.Array("entries").Struct(-1).Struct("header").Data("tag").Set(17);
                dut.Step(1);
                assert dut.record_out.Array("entries").Struct(-1).Struct("header").Data("tag").U().longValue() == 17;
            } else {
                boolean rejected = false;
                try { dut.GetInternalSignalView("record_in"); }
                catch (RuntimeException error) {
                    if (!error.getMessage().contains("__VPI_STRUCT_ERROR__")) throw error;
                    rejected = true;
                }
                assert rejected;
            }
            assert dut.GetInternalSignalView("packed_in").Struct().Data("valid").CSelf().equals(cached.Data("valid").CSelf());
            assert cached.Data("valid").U().longValue() == 1;
            dut.Finish();
            return;
        }
        XView heldView = dut.GetInternalSignalView("packed_in");
        boolean wrongKind = false;
        try { heldView.Array(); } catch (RuntimeException expected) { wrongKind = true; }
        assert wrongKind;
        XStruct packed = heldView.Struct();
        assert packed.Data("tag").S().longValue() == -3;
        assert packed.Data("valid").U().longValue() == 1;
        XArray matrix = dut.GetInternalSignalView("matrix").Array();
        assert matrix.Array(1).Data(-2).U().longValue() == 1;
        matrix.Array(1).Data(-1).Set(1);
        XArray records = dut.GetInternalSignalView("packed_records").Array();
        records.Struct(1).Data("valid").Set(1);
        dut.Step(1);
        assert dut.matrix_out.U().longValue() == 0xc1;
        assert dut.packed_records_out.Struct(1).Data("valid").U().longValue() == 1;
        dut.Finish();
        boolean expired = false;
        try { heldView.Struct(); } catch (RuntimeException expected) { expired = true; }
        assert expired;
    }
}
'''

GO_VPI_CHECK = r'''
package main
import (
    "fmt"
    "os"
    "strings"
    ut "UT_AggregatePorts"
    xs "xspcomm"
)
func mustPanic(action func()) {
    failed := false
    func() {
        defer func() { failed = recover() != nil }()
        action()
    }()
    if !failed { panic("expected a checked XView error") }
}
func main() {
    dut := ut.NewUT_AggregatePorts()
    dut.Packed_in.Set(59)
    dut.Matrix.Set(0x81)
    dut.Record_in.Data("enabled").SetGo(1)
    dut.Step(1)
    if len(os.Args) > 1 && os.Args[1] == "unpacked-struct" {
        cachedView := dut.GetInternalSignalView("packed_in")
        defer xs.DeleteXView(cachedView)
        cached := cachedView.Struct()
        defer xs.DeleteXStruct(cached)
        if __VPI_STRUCT_SUPPORTED__ {
            recordView := dut.GetInternalSignalView("record_in")
            defer xs.DeleteXView(recordView)
            record := recordView.Struct()
            defer xs.DeleteXStruct(record)
            if record.Data("enabled").U64() != 1 { panic("VPI struct read mismatch") }
            record.Array("entries").Struct(-1).Struct("header").Data("tag").SetGo(17)
            dut.Step(1)
            if dut.Record_out.Array("entries").Struct(-1).Struct("header").Data("tag").U64() != 17 { panic("VPI struct write mismatch") }
        } else {
            var rejected interface{}
            func() {
                defer func() { rejected = recover() }()
                view := dut.GetInternalSignalView("record_in")
                xs.DeleteXView(view)
            }()
            if rejected == nil || !strings.Contains(fmt.Sprint(rejected), "__VPI_STRUCT_ERROR__") {
                panic(fmt.Sprintf("expected the missing VPI member diagnostic, got %v", rejected))
            }
        }
        if cached.Data("valid").U64() != 1 { panic("cached VPI view was invalidated") }
        dut.Finish()
        return
    }
    heldView := dut.GetInternalSignalView("packed_in")
    defer xs.DeleteXView(heldView)
    mustPanic(func() { heldView.Array() })
    packed := heldView.Struct()
    if packed.Data("tag").S64() != -3 || packed.Data("valid").U64() != 1 { panic("packed VPI mismatch") }
    matrix := dut.GetInternalSignalView("matrix").Array()
    if matrix.Array(1).Data(-2).U64() != 1 { panic("VPI array-index mismatch") }
    matrix.Array(1).Data(-1).SetGo(1)
    records := dut.GetInternalSignalView("packed_records").Array()
    records.Struct(1).Data("valid").SetGo(1)
    dut.Step(1)
    if dut.Matrix_out.U().Uint64() != 0xc1 { panic("VPI packed-array write mismatch") }
    if dut.Packed_records_out.Struct(1).Data("valid").U64() != 1 { panic("VPI unpacked-array write mismatch") }
    dut.Finish()
    mustPanic(func() { heldView.Struct() })
}
'''

LUA_VPI_CHECK = r'''
package.path = "UT_AggregatePorts/?.lua;" .. package.path
local DUT = require("UT_AggregatePorts")
local dut = DUT:new()
dut.packed_in:Set(59)
dut.matrix:Set(0x81)
dut.record_in:Data("enabled"):Set(1)
dut:Step(1)
if arg[1] == "unpacked-struct" then
    local cached = dut:GetInternalSignalView("packed_in"):Struct()
    if __VPI_STRUCT_SUPPORTED__ then
        local record = dut:GetInternalSignalView("record_in"):Struct()
        assert(record:Data("enabled"):U() == 1)
        record:Array("entries"):Struct(-1):Struct("header"):Data("tag"):Set(17)
        dut:Step(1)
        assert(dut.record_out:Array("entries"):Struct(-1):Struct("header"):Data("tag"):U() == 17)
    else
        local ok, error = pcall(function() dut:GetInternalSignalView("record_in") end)
        assert(not ok and tostring(error):find("__VPI_STRUCT_ERROR__", 1, true), tostring(error))
    end
    assert(cached:Data("valid"):U() == 1)
    dut:Finish()
    return
end
local held_view = dut:GetInternalSignalView("packed_in")
local ok = pcall(function() held_view:Array() end)
assert(not ok)
local packed = held_view:Struct()
assert(packed:Data("tag"):S() == -3)
assert(packed:Data("valid"):U() == 1)
local matrix = dut:GetInternalSignalView("matrix"):Array()
assert(matrix:Array(1):Data(-2):U() == 1)
matrix:Array(1):Data(-1):Set(1)
local records = dut:GetInternalSignalView("packed_records"):Array()
records:Struct(1):Data("valid"):Set(1)
dut:Step(1)
assert(dut.matrix_out:U() == 0xc1)
assert(dut.packed_records_out:Struct(1):Data("valid"):U() == 1)
dut:Finish()
ok = pcall(function() held_view:Struct() end)
assert(not ok)
dut:Finish()
collectgarbage()
'''

SCALA_VPI_CHECK = r'''
package com.ut
import com.ut.AggregatePorts.UT_AggregatePorts
object example {
    def main(args: Array[String]): Unit = {
        val dut = new UT_AggregatePorts()
        dut.packed_in.Set(59)
        dut.matrix.Set(0x81)
        dut.record_in.Data("enabled").Set(1)
        dut.Step(1)
        if (args.headOption.contains("unpacked-struct")) {
            val cached = dut.GetInternalSignalView("packed_in").Struct()
            if (__VPI_STRUCT_SUPPORTED__) {
                val record = dut.GetInternalSignalView("record_in").Struct()
                assert(record.Data("enabled").U().longValue() == 1)
                record.Array("entries").Struct(-1).Struct("header").Data("tag").Set(17)
                dut.Step(1)
                assert(dut.record_out.Array("entries").Struct(-1).Struct("header").Data("tag").U().longValue() == 17)
            } else {
                var rejected = false
                try { dut.GetInternalSignalView("record_in") }
                catch {
                    case error: RuntimeException =>
                        assert(error.getMessage.contains("__VPI_STRUCT_ERROR__"))
                        rejected = true
                }
                assert(rejected)
            }
            assert(cached.Data("valid").U().longValue() == 1)
            dut.Finish()
            return
        }
        val packed = dut.GetInternalSignalView("packed_in").Struct()
        assert(packed.Data("tag").S().longValue() == -3)
        assert(packed.Data("valid").U().longValue() == 1)
        val matrix = dut.GetInternalSignalView("matrix").Array()
        assert(matrix.Array(1).Data(-2).U().longValue() == 1)
        matrix.Array(1).Data(-1).Set(1)
        val records = dut.GetInternalSignalView("packed_records").Array()
        records.Struct(1).Data("valid").Set(1)
        dut.Step(1)
        assert(dut.matrix_out.U().longValue() == 0xc1)
        assert(dut.packed_records_out.Struct(1).Data("valid").U().longValue() == 1)
        dut.Finish()
    }
}
'''


UNPACKED_STRUCT_VPI_ERROR = (
    "Simulator does not identify the VPI object as a struct or expose its "
    "RTL-declared member: record_in.enabled"
)


class AggregateViewsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).resolve().parents[2]
        cls.picker = os.environ.get("PICKER_BIN", str(cls.root / "build/bin/picker"))
        if not Path(cls.picker).is_file() or not shutil.which("verilator"):
            raise unittest.SkipTest("Built Picker and Verilator are required")
        version_text = subprocess.check_output(["verilator", "--version"], text=True)
        version = re.search(r"Verilator (\d+)\.(\d+)", version_text)
        if version is None:
            raise RuntimeError(f"Cannot determine Verilator version: {version_text}")
        cls.verilator_version = tuple(map(int, version.groups()))
        cls.vpi_builds = {}

    def run_command(self, args, *, cwd=None, env=None):
        result = subprocess.run(args, cwd=cwd, env=env, text=True, capture_output=True, timeout=600)
        self.assertEqual(result.returncode, 0, f"{args}\n{result.stdout[-8000:]}\n{result.stderr[-12000:]}")
        return result.stdout

    def require_scala_sdk_compatibility(self):
        if not shutil.which("scalac"):
            self.skipTest("Scala compiler unavailable")
        library = self.run_command([self.picker, "--show_xcom_lib_location_scala"]).strip().splitlines()[-1]
        with tempfile.TemporaryDirectory() as directory:
            source = Path(directory) / "ScalaSdkCompatibility.scala"
            source.write_text("import com.xspcomm.BaseDUTTrait\nobject ScalaSdkCompatibility { val t = classOf[BaseDUTTrait] }\n")
            result = subprocess.run(["scalac", "-cp", library, "-d", directory, str(source)],
                                    text=True, capture_output=True)
        diagnostic = result.stdout + result.stderr
        if result.returncode and "Scala signature package has wrong version" in diagnostic:
            self.skipTest("Installed scalac is binary-incompatible with the xcomm Scala SDK")
        self.assertEqual(result.returncode, 0, diagnostic)

    def exercise(self, language):
        for access in os.environ.get("AGGREGATE_ACCESS", "dpi,mem_direct").split(","):
            with self.subTest(language=language, access=access), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                output = base / "AggregatePorts"
                self.run_command([
                    self.picker, "export", str(self.root / "test/data/aggregate_views.sv"),
                    "--sname", "AggregatePorts", "--sdir", str(self.root / "template"),
                    "--tdir", str(output), "--sim", "verilator", "--lang", language,
                    "--rw", access, "--autobuild", "false", "-V", "-Wno-fatal",
                ])
                wrapper = (output / "AggregatePorts_top.sv").read_text()
                self.assertNotIn("logic _picker_leaf_", wrapper)
                self.assertIn("type(AggregatePorts.record_in)", wrapper)
                self.assertNotIn("_picker_array_value", wrapper)
                self.assertNotIn("_picker_index_", wrapper)
                cmake = (output / "cmake/verilator.cmake").read_text()
                self.assertIn('VERSION_LESS "5.048"', cmake)
                if access == "dpi":
                    # Independent fields share indexed access across array values.
                    self.assertEqual(wrapper.count('export "DPI-C" function get_'), 14)
                else:
                    self.assertNotIn('export "DPI-C" function get_', wrapper)
                if language == "cpp":
                    (output / "cpp/example.cpp").write_text(CPP_CHECK)
                elif language == "java":
                    (output / "java/example.java").write_text(JAVA_CHECK)
                elif language == "golang":
                    (output / "golang/example.go").write_text(GO_CHECK)
                elif language == "lua":
                    (output / "lua/example.lua").write_text(LUA_CHECK)
                elif language == "scala":
                    (output / "scala/example.scala").write_text(SCALA_CHECK)
                self.run_command(["make", "-C", str(output), "NPROC=4"])
                if language == "python":
                    self.run_command([sys.executable, "-c", PYTHON_CHECK], cwd=base)
                elif language == "cpp":
                    self.run_command([str(output / "UTAggregatePorts_example")], cwd=output)
                elif language == "java":
                    jar = self.run_command([self.picker, "--show_xcom_lib_location_java"]).strip().splitlines()[-1]
                    self.run_command(["java", "-ea", "-cp", os.pathsep.join([
                        jar, str(output / "UT_AggregatePorts-java.jar")]), "com.ut.example"], cwd=output)
                elif language == "golang":
                    library = self.run_command([self.picker, "--show_xcom_lib_location_golang"]).strip().splitlines()[-1]
                    self.run_command(["go", "run", "example.go"], cwd=output,
                        env={**os.environ, "GO111MODULE": "off", "GOPATH": os.pathsep.join([
                            str(output / "golang"), library])})
                elif language == "lua":
                    self.run_command(["lua", "example.lua"], cwd=output)
                else:
                    jar = self.run_command([self.picker, "--show_xcom_lib_location_scala"]).strip().splitlines()[-1]
                    self.run_command(["scala", "-cp", os.pathsep.join([
                        jar, str(output / "UT_AggregatePorts-scala.jar")]), "com.ut.example"], cwd=output)

    def test_python(self):
        self.exercise("python")

    def test_vpi_python(self):
        self.exercise_vpi_language("python", VPI_CHECK)

    def test_vpi_cpp(self):
        self.exercise_vpi_language("cpp", CPP_VPI_CHECK)

    def exercise_vpi_language(self, language, check, case="basic"):
        if language in ("golang", "lua", "scala") and not os.environ.get("AGGREGATE_EXTRA_LANGUAGES"):
            self.skipTest("Set AGGREGATE_EXTRA_LANGUAGES=1 to test Go/Lua/Scala toolchains")
        if language == "java" and not shutil.which("javac"):
            self.skipTest("Java toolchain unavailable")
        if language == "scala":
            self.require_scala_sdk_compatibility()
        if language not in self.vpi_builds:
            # Share only build artifacts: capability cases run in separate
            # processes and report independently, without doubling model builds.
            directory = tempfile.TemporaryDirectory(prefix="picker-vpi-")
            self.addClassCleanup(directory.cleanup)
            base = Path(directory.name)
            output = base / "AggregatePorts"
            supported = self.verilator_version >= (5, 52)
            check = check.replace("__VPI_STRUCT_SUPPORTED__",
                                  str(supported) if language == "python" else str(supported).lower())
            check = check.replace("__VPI_STRUCT_ERROR__", UNPACKED_STRUCT_VPI_ERROR)
            self.run_command([
                self.picker, "export", str(self.root / "test/data/aggregate_views.sv"),
                "--sname", "AggregatePorts", "--sdir", str(self.root / "template"),
                "--tdir", str(output), "--sim", "verilator", "--lang", language,
                "--rw", "dpi", "--vpi", "--autobuild", "false",
            ])
            if language != "python":
                extension = {"cpp": "cpp", "java": "java", "golang": "go",
                             "lua": "lua", "scala": "scala"}[language]
                (output / language / f"example.{extension}").write_text(check)
            self.run_command(["make", "-C", str(output), "NPROC=4"])
            cwd, env = output, None
            if language == "python":
                command, cwd = [sys.executable, "-c", check], base
            elif language == "cpp":
                command = [str(output / "UTAggregatePorts_example")]
            elif language == "java":
                jar = self.run_command([self.picker, "--show_xcom_lib_location_java"]).strip().splitlines()[-1]
                command = ["java", "-ea", "-cp", os.pathsep.join([
                    jar, str(output / "UT_AggregatePorts-java.jar")]), "com.ut.example"]
            elif language == "golang":
                library = self.run_command([self.picker, "--show_xcom_lib_location_golang"]).strip().splitlines()[-1]
                command = ["go", "run", "example.go"]
                env = {**os.environ, "GO111MODULE": "off", "GOPATH": os.pathsep.join([
                    str(output / "golang"), library])}
            elif language == "lua":
                command = ["lua", "example.lua"]
            else:
                jar = self.run_command([self.picker, "--show_xcom_lib_location_scala"]).strip().splitlines()[-1]
                command = ["scala", "-cp", os.pathsep.join([
                    jar, str(output / "UT_AggregatePorts-scala.jar")]), "com.ut.example"]
            self.vpi_builds[language] = command, cwd, env
        command, cwd, env = self.vpi_builds[language]
        self.run_command([*command, case], cwd=cwd, env=env)

    def test_vpi_java(self):
        self.exercise_vpi_language("java", JAVA_VPI_CHECK)

    def test_vpi_golang(self):
        self.exercise_vpi_language("golang", GO_VPI_CHECK)

    def test_vpi_lua(self):
        self.exercise_vpi_language("lua", LUA_VPI_CHECK)

    def test_vpi_scala(self):
        self.exercise_vpi_language("scala", SCALA_VPI_CHECK)

    def test_vpi_unpacked_struct_python(self):
        self.exercise_vpi_language("python", VPI_CHECK, "unpacked-struct")

    def test_vpi_unpacked_struct_cpp(self):
        self.exercise_vpi_language("cpp", CPP_VPI_CHECK, "unpacked-struct")

    def test_vpi_unpacked_struct_java(self):
        self.exercise_vpi_language("java", JAVA_VPI_CHECK, "unpacked-struct")

    def test_vpi_unpacked_struct_golang(self):
        self.exercise_vpi_language("golang", GO_VPI_CHECK, "unpacked-struct")

    def test_vpi_unpacked_struct_lua(self):
        self.exercise_vpi_language("lua", LUA_VPI_CHECK, "unpacked-struct")

    def test_vpi_unpacked_struct_scala(self):
        self.exercise_vpi_language("scala", SCALA_VPI_CHECK, "unpacked-struct")

    def test_integral_struct_fields(self):
        rtl = """package IntegralTypes;
typedef enum logic [1:0] { IDLE=0, RUN=2 } state_t;
typedef struct { state_t state; int count; bit ready; } control_t;
endpackage
module IntegralPorts(input var IntegralTypes::control_t req,
                     output var IntegralTypes::control_t rsp);
assign rsp=req;
endmodule
"""
        check = """from IntegralPorts import DUTIntegralPorts
dut = DUTIntegralPorts()
assert dut.req.Field('state').IsFourState()
assert not dut.req.Field('count').IsFourState()
assert dut.req.Field('count').IsSigned()
dut.req.state.value = 2
dut.req.count.value = -9
dut.req.ready.value = 1
dut.Step(1)
assert dut.rsp.state.value == 2
assert dut.rsp.count.S() == -9
assert dut.rsp.ready.value == 1
dut.Finish()
"""
        for access in os.environ.get("AGGREGATE_ACCESS", "dpi,mem_direct").split(","):
            with self.subTest(access=access), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                source = base / "integral.sv"
                source.write_text(rtl)
                output = base / "IntegralPorts"
                self.run_command([self.picker, "export", str(source), "--sname", "IntegralPorts",
                    "--sdir", str(self.root / "template"), "--tdir", str(output), "--sim", "verilator",
                    "--lang", "python", "--rw", access, "--autobuild", "false"])
                self.run_command(["make", "-C", str(output), "NPROC=4"])
                self.run_command([sys.executable, "-c", check], cwd=base)

    def test_nested_packed_and_mixed_arrays(self):
        rtl = """package ComposedTypes;
typedef struct packed { logic [2:0] tag; bit valid; } header_t;
typedef struct packed {
    header_t header;
    header_t [1:0] slots;
    logic [0:1][4:2] matrix;
} body_t;
typedef struct {
    logic [3:0] narrow;
    header_t items [2:1];
    struct { logic [8:0] value; } child;
} frame_t;
endpackage
module Composed(
    input ComposedTypes::body_t body, output ComposedTypes::body_t body_out,
    input ComposedTypes::header_t [2:1][0:1] table_in,
    output ComposedTypes::header_t [2:1][0:1] table_out,
    input logic [1:0][5:3] lanes [-1:0], output logic [1:0][5:3] lanes_out [-1:0],
    input var ComposedTypes::frame_t frame, output var ComposedTypes::frame_t frame_out
);
assign body_out=body;
assign table_out=table_in;
assign lanes_out=lanes;
assign frame_out=frame;
endmodule
"""
        check = """from Composed import DUTComposed
dut = DUTComposed()
for n in (0, 1):
    prefix = f'Composed_{n}_'
    body = getattr(dut, prefix + 'body_view')
    body.header.Data('tag').value = 5 + n
    body.Array('slots')[1].Data('tag').value = 3 + n
    body.Array('slots')[0].Data('valid').value = 1
    body.Array('matrix')[0][4].value = 1
    table = getattr(dut, prefix + 'table_in_view')
    assert table.bounds == ((2, 1), (0, 1))
    table[2][0].Data('tag').value = 6 - n
    table[1][1].Data('valid').value = 1
    lanes = getattr(dut, prefix + 'lanes')
    assert lanes.shape == (2,)
    lanes[-1].value = 0x21 + n
    lanes.Array(0)[1][4].value = 1
    frame = getattr(dut, prefix + 'frame')
    frame.narrow.value = n + 7
    frame.items[2].Data('tag').value = n + 2
    frame.items[1].Data('valid').value = 1
    frame.child['value'].value = n + 0x123
count = dut.xport.PortCount()
dut.Step(1)
for n in (0, 1):
    prefix = f'Composed_{n}_'
    body = getattr(dut, prefix + 'body_out_view')
    assert body.header.Data('tag').value == 5 + n
    assert body.Array('slots')[1].Data('tag').value == 3 + n
    assert body.Array('slots')[0].Data('valid').value == 1
    assert body.Array('matrix')[0][4].value == 1
    table = getattr(dut, prefix + 'table_out_view')
    assert table[2][0].Data('tag').value == 6 - n
    assert table[1][1].Data('valid').value == 1
    lanes = getattr(dut, prefix + 'lanes_out')
    assert lanes[-1].value == 0x21 + n
    assert lanes.Array(0)[1][4].value == 1
    frame = getattr(dut, prefix + 'frame_out')
    assert frame.narrow.value == n + 7
    assert frame.items[2].Data('tag').value == n + 2
    assert frame.items[1].Data('valid').value == 1
    assert frame.child['value'].value == n + 0x123
assert dut.xport.PortCount() == count == 32
dut.Finish()
"""
        for access in os.environ.get("AGGREGATE_ACCESS", "dpi,mem_direct").split(","):
            with self.subTest(access=access), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                source = base / "composed.sv"
                source.write_text(rtl)
                output = base / "Composed"
                self.run_command([self.picker, "export", str(source), "--sname", "Composed,2",
                    "--tname", "Composed", "--sdir", str(self.root / "template"), "--tdir", str(output),
                    "--sim", "verilator", "--lang", "python", "--rw", access, "--autobuild", "false",
                    "-V", "-Wno-fatal"])
                self.run_command(["make", "-C", str(output), "NPROC=4"])
                self.run_command([sys.executable, "-c", check], cwd=base)

    def test_view_names_do_not_shadow_ports_or_bookkeeping(self):
        rtl = """typedef struct packed { logic Data; logic value; } packet_t;
module NamePorts(input packet_t packet, input logic packet_view,
                 input logic packet_view_, input logic _picker_views);
endmodule
"""
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            source = base / "names.sv"
            source.write_text(rtl)
            output = base / "NamePorts"
            self.run_command([self.picker, "export", str(source), "--sname", "NamePorts",
                "--sdir", str(self.root / "template"), "--tdir", str(output), "--sim", "verilator",
                "--lang", "python", "--autobuild", "false"])
            wrapper = (output / "python/dut.py").read_text()
            self.assertIn("self.packet_view__ = xsp.XView.FromPort", wrapper)
            self.assertIn("self.pin__picker_views = xsp.XData", wrapper)
            self.assertNotIn("self._picker_views = xsp.XData", wrapper)

    def test_unsupported_nested_types_fail_before_generation(self):
        cases = (
            "typedef struct { logic queue[$]; } item_t;",
            "typedef struct { string name; } item_t;",
            "typedef union packed { logic [7:0] a; bit [7:0] b; } item_t;",
        )
        for declaration in cases:
            with self.subTest(declaration=declaration), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                source = base / "unsupported.sv"
                source.write_text(declaration + "\nmodule Bad(input var item_t item); endmodule\n")
                output = base / "Bad"
                result = subprocess.run([self.picker, "export", str(source), "--sname", "Bad",
                    "--sdir", str(self.root / "template"), "--tdir", str(output), "--sim", "verilator",
                    "--lang", "python", "--autobuild", "false"], capture_output=True, text=True, timeout=60)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("unsupported type", result.stderr)
                self.assertFalse(output.exists())

    def test_cpp(self):
        self.exercise("cpp")

    def test_generated_build_rejects_older_verilator(self):
        for access in ("dpi", "mem_direct"):
            with self.subTest(access=access), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                output = base / "AggregatePorts"
                self.run_command([self.picker, "export", str(self.root / "test/data/aggregate_views.sv"),
                    "--sname", "AggregatePorts", "--sdir", str(self.root / "template"),
                    "--tdir", str(output), "--lang", "python", "--rw", access, "--autobuild", "false"])
                old = base / "old-verilator"
                old.mkdir()
                (old / "verilator-config.cmake").write_text('set(verilator_VERSION "5.026")\n')
                (old / "verilator-config-version.cmake").write_text(
                    'set(PACKAGE_VERSION "5.026")\nset(PACKAGE_VERSION_COMPATIBLE TRUE)\n')
                binary = old / "verilator"
                binary.write_text(f'#!/bin/sh\necho "VERILATOR_ROOT = {old}"\n')
                binary.chmod(0o755)
                result = subprocess.run(["cmake", "-S", str(output), "-B", str(base / "build"),
                    "-DSIMULATOR=verilator", "-DPROJECT=AggregatePorts"], text=True, capture_output=True,
                    timeout=60, env={**os.environ, "PATH": str(old) + os.pathsep + os.environ["PATH"]})
                self.assertNotEqual(result.returncode, 0)
                diagnostic = " ".join((result.stdout + result.stderr).split())
                self.assertIn("require Verilator 5.048 or newer", diagnostic)

    def test_java(self):
        if not shutil.which("javac"):
            self.skipTest("Java toolchain unavailable")
        self.exercise("java")

    def test_golang(self):
        if not os.environ.get("AGGREGATE_EXTRA_LANGUAGES"):
            self.skipTest("Set AGGREGATE_EXTRA_LANGUAGES=1 to test Go/Lua/Scala toolchains")
        self.exercise("golang")

    def test_lua(self):
        if not os.environ.get("AGGREGATE_EXTRA_LANGUAGES"):
            self.skipTest("Set AGGREGATE_EXTRA_LANGUAGES=1 to test Go/Lua/Scala toolchains")
        self.exercise("lua")

    def test_scala(self):
        if not os.environ.get("AGGREGATE_EXTRA_LANGUAGES"):
            self.skipTest("Set AGGREGATE_EXTRA_LANGUAGES=1 to test Go/Lua/Scala toolchains")
        self.require_scala_sdk_compatibility()
        self.exercise("scala")


if __name__ == "__main__":
    unittest.main()
