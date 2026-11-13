"""Build and simulate unpacked array views with RTL-indexed XData elements."""

import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import unittest


RTL = """
module Arrays #(parameter W = 5, N = 3) (
    input logic [W-1:0] data [N+1:2],
    output logic [W-1:0] result [2:N+1],
    input bit flag [1:0],
    output bit flag_out [0:1],
    input bit signed [95:0] wide [3:2],
    output bit signed [95:0] wide_out [2:3],
    input logic signed [0:4] matrix [-1:0][3:2],
    output logic signed [0:4] matrix_out [-1:0][2:3],
    input logic lone [7:7],
    output logic lone_out [7:7],
    input logic [7:0] plain,
    output logic [7:0] plain_out,
    input logic [-1:-5] negative [3:2],
    output logic [-1:-5] negative_out [2:3],
    input logic [7:0] cube [3:2][-1:0][4:6],
    output logic [7:0] cube_out [2:3][0:-1][6:4],
    input logic data_0,
    output logic data_0_out,
    input logic _picker_array_0,
    output logic private_out
);
    for (genvar i = 2; i <= N+1; i++) begin
        assign result[i] = data[i] ^ W'(i);
    end
    for (genvar i = 0; i < 2; i++) begin
        assign flag_out[i] = ~flag[i];
    end
    for (genvar i = 2; i <= 3; i++) begin
        assign wide_out[i] = ~wide[i];
        assign negative_out[i] = ~negative[i];
    end
    for (genvar i = -1; i <= 0; i++) begin
        for (genvar j = 2; j <= 3; j++) begin
            assign matrix_out[i][j] = matrix[i][j] ^ 5'(j);
        end
    end
    assign lone_out[7] = ~lone[7];
    assign plain_out = ~plain;
    assign data_0_out = ~data_0;
    assign private_out = ~_picker_array_0;
    for (genvar i = 2; i <= 3; ++i)
        for (genvar j = -1; j <= 0; ++j)
            for (genvar k = 4; k <= 6; ++k)
                assign cube_out[i][j][k] = cube[i][j][k] ^ 8'(i * 16 + j * 4 + k);
endmodule
"""

CHECK = """
import os
import random
import sys
from Arrays import DUTArrays, xsp

rng = random.Random(42)
dut = DUTArrays()
try:
    cases = []
    for prefix in sys.argv[1:] or ['']:
        def port(name):
            value = getattr(dut, prefix + name)
            by_name = dut[prefix + name]
            if hasattr(value, 'CSelf'):
                assert by_name.CSelf() == value.CSelf()
            else:
                assert by_name is value
            return value

        def add(source, target, width, xor, source_name, target_name):
            assert dut[prefix + source_name].CSelf() == source.CSelf()
            assert dut[prefix + target_name].CSelf() == target.CSelf()
            cases.append((source, target, width, xor))
            backend = xsp.XDataBackendKind_MemDirect if os.environ['ARRAY_ACCESS'] == 'mem_direct' else xsp.XDataBackendKind_DPI
            assert source.GetBackendKind() == target.GetBackendKind() == backend
            assert source.IsArrayBound() == target.IsArrayBound() == ('[' in source_name)

        data, result = port('data'), port('result')
        matrix, matrix_out = port('matrix'), port('matrix_out')
        assert data.bounds == ((4, 2),) and data.shape == (3,)
        assert list(data.indices) == [4, 3, 2]
        assert list(data) == [data[4], data[3], data[2]] and len(data) == 3
        assert matrix.bounds == ((-1, 0), (3, 2)) and matrix.shape == (2, 2)
        assert list(matrix.indices) == [-1, 0]
        assert matrix[-1].bounds == ((3, 2),) and matrix[-1].shape == (2,)
        assert list(matrix[-1].indices) == [3, 2]
        assert not hasattr(dut, prefix + 'data_4')
        for array, index in ((data, -1), (data, 1), (data, 5), (matrix, -2), (matrix, 1), (matrix[-1], 4)):
            try:
                array[index]
            except IndexError:
                pass
            else:
                raise AssertionError('out-of-range RTL index was accepted')
        for index in (1.5, '4', slice(None)):
            try:
                data[index]
            except TypeError:
                pass
            else:
                raise AssertionError('non-integer array index was accepted')
        try:
            setattr(dut, prefix + 'data', [])
        except AssertionError:
            pass
        else:
            raise AssertionError('array port was replaced')
        try:
            data[4] = 0
        except TypeError:
            pass
        else:
            raise AssertionError('array element was replaced')

        for i in data.indices:
            add(data[i], result[i], 5, i, f'data[{i}]', f'result[{i}]')
        for i in range(2):
            add(port('flag')[i], port('flag_out')[i], 1, 1, f'flag[{i}]', f'flag_out[{i}]')
        for i in range(2, 4):
            add(port('wide')[i], port('wide_out')[i], 96, (1 << 96) - 1, f'wide[{i}]', f'wide_out[{i}]')
            add(port('negative')[i], port('negative_out')[i], 5, 31, f'negative[{i}]', f'negative_out[{i}]')
        for i in matrix.indices:
            for j in matrix[i].indices:
                add(matrix[i][j], matrix_out[i][j], 5, j, f'matrix[{i}][{j}]', f'matrix_out[{i}][{j}]')
        cube, cube_out = port('cube'), port('cube_out')
        assert cube.shape == (2, 2, 3)
        for i in cube.indices:
            for j in cube[i].indices:
                for k in cube[i][j].indices:
                    add(cube[i][j][k], cube_out[i][j][k], 8, i * 16 + j * 4 + k,
                        f'cube[{i}][{j}][{k}]', f'cube_out[{i}][{j}][{k}]')
        add(port('lone')[7], port('lone_out')[7], 1, 1, 'lone[7]', 'lone_out[7]')
        for source, target, width in (('plain', 'plain_out', 8), ('data_0', 'data_0_out', 1), ('_picker_array_0', 'private_out', 1)):
            add(port(source), port(target), width, (1 << width) - 1, source, target)
    for immediate in (False, True):
        if immediate:
            for source, _, _, _ in cases:
                source.AsImmWrite()
        for _ in range(32):
            expected = {}
            for source, target, width, xor in cases:
                value = rng.getrandbits(width)
                source.value = value
                expected[id(target)] = (target, value ^ xor)
            if immediate:
                tick = dut.GetXClock().GetHalfTick()
                dut.RefreshComb()
                assert dut.GetXClock().GetHalfTick() == tick
            else:
                dut.Step(1)
            for target, value in expected.values():
                assert target.value == value, (target, target.value, value)
    if os.environ['ARRAY_ACCESS'] == 'dpi':
        # Cached accessor pointers must belong to the correct model/library instance.
        other = DUTArrays()
        try:
            first = getattr(dut, (sys.argv[1:] or [''])[0] + 'data')[4]
            second = getattr(other, (sys.argv[1:] or [''])[0] + 'data')[4]
            first.AsImmWrite()
            second.AsImmWrite()
            first.value, second.value = 5, 17
            dut.RefreshComb()
            assert first.value == 5 and second.value == 17
            other.RefreshComb()
            assert first.value == 5 and second.value == 17
        finally:
            other.Finish()
finally:
    dut.Finish()
"""

CPP_CHECK = """
#include "UT_Arrays.hpp"
#include <cassert>

int main() {
    UTArrays dut;
    assert(dut.data[4].GetBackendKind() == XDataBackendKind::EXPECTED_BACKEND);
    assert(dut.data[4].IsArrayBound() && dut.cube[3][-1][5].IsArrayBound());
    assert(dut.matrix_out[-1][2].GetBackendKind() == XDataBackendKind::EXPECTED_BACKEND);
    assert(dut.data.left() == 4 && dut.data.right() == 2 && dut.data.size() == 3);
    assert(dut.matrix.left() == -1 && dut.matrix[-1].left() == 3);
    assert(dut.xport["data[4]"].CSelf() == dut.data[4].CSelf());
    const auto &matrix = dut.matrix;
    assert(matrix[-1][2].mWidth == 5);
    bool caught = false;
    try { dut.data[-1]; } catch (const std::out_of_range &) { caught = true; }
    assert(caught);
    caught = false;
    try { dut.matrix[-1][4]; } catch (const std::out_of_range &) { caught = true; }
    assert(caught);
    for (int n = 0; n < 32; ++n) {
        for (int i = 2; i <= 4; ++i) { dut.data[i].Set((n + i) & 31); }
        dut.matrix[-1][2].Set(n);
        dut.negative[3].Set(n);
        dut.cube[3][-1][5].Set(n);
        dut.data_0.Set(n & 1);
        dut._picker_array_0.Set((n >> 1) & 1);
        dut.Step(1);
        for (int i = 2; i <= 4; ++i) { assert(dut.result[i].U() == (((n + i) & 31) ^ i)); }
        assert(dut.matrix_out[-1][2].U() == (n ^ 2));
        assert(dut.negative_out[3].U() == (n ^ 31));
        assert(dut.cube_out[3][-1][5].U() == (n ^ 49));
        assert(dut.data_0_out.U() == ((n & 1) ^ 1));
        assert(dut.private_out.U() == (((n >> 1) & 1) ^ 1));
    }
}
"""

JAVA_CHECK = """
package com.ut;
import com.ut.Arrays.UT_Arrays;

public class example {
    public static void main(String[] args) {
        UT_Arrays dut = new UT_Arrays();
        try {
            assert dut.data.size() == 3;
            assert dut.matrix.containsKey(-1);
            assert !dut.data.containsKey(-1);
            assert dut.data.get(4).IsArrayBound();
            boolean caught = false;
            try { dut.data.put(4, dut.data.get(3)); }
            catch (UnsupportedOperationException e) { caught = true; }
            assert caught;
            for (int n = 0; n < 32; ++n) {
                for (int i = 2; i <= 4; ++i) { dut.data.get(i).Set((n + i) & 31); }
                dut.matrix.get(-1).get(2).Set(n);
                dut.negative.get(3).Set(n);
                dut.cube.get(3).get(-1).get(5).Set(n);
                dut.data_0.Set(n & 1);
                dut._picker_array_0.Set((n >> 1) & 1);
                dut.Step();
                for (int i = 2; i <= 4; ++i) { assert dut.result.get(i).U().intValue() == (((n + i) & 31) ^ i); }
                assert dut.matrix_out.get(-1).get(2).U().intValue() == (n ^ 2);
                assert dut.negative_out.get(3).U().intValue() == (n ^ 31);
                assert dut.cube_out.get(3).get(-1).get(5).U().intValue() == (n ^ 49);
                assert dut.data_0_out.U().intValue() == ((n & 1) ^ 1);
                assert dut.private_out.U().intValue() == (((n >> 1) & 1) ^ 1);
            }
        } finally { dut.Finish(); }
    }
}
"""

INTERNAL_RTL = """
module InternalArrays (
    input logic row,
    input logic [1:0] col,
    output logic [64:0] value,
    output logic [63:0] word,
    output logic flag
);
    logic [64:0] mem [2:3][-1:1] /*verilator public_flat_rw*/;
    logic [63:0] words [0:2] /*verilator public_flat_rw*/;
    bit tiny [4:2] /*verilator public_flat_rw*/;
    initial begin
        foreach (mem[i, j]) mem[i][j] = 65'(i * 10 + j);
        foreach (words[i]) words[i] = 64'(i);
        foreach (tiny[i]) tiny[i] = 0;
    end
    assign value = mem[2 + int'(row)][int'(col) - 1];
    assign word = words[col];
    assign flag = tiny[2 + int'(col)];
endmodule
"""

INTERNAL_CHECK = """
import ctypes
import sys
from InternalArrays import DUTInternalArrays, xsp

dut = DUTInternalArrays()
try:
    dut.row.AsImmWrite()
    dut.col.AsImmWrite()
    dut.Step(1)
    names = dut.GetInternalSignalList()
    def find(suffix):
        return next(name for name in names if name.endswith('.' + suffix))
    name = find('mem')
    cfg = dut.xcfg.At(name)
    assert cfg.array_size == 6 and cfg.mem_bytes == 12 and cfg.rtl_width == 65
    memory = dut.GetInternalSignal(name, is_array=True)
    array_view = dut.xcfg.NewXDataArrayView(name)
    assert tuple(array_view.Shape()) == (2, 3)
    assert tuple(array_view.Strides()) == (36, 12)
    assert array_view.FlatIndex((1, 2)) == 5
    assert array_view.Address(5) == dut.xcfg.Address(name) + 5 * 12
    assert len(memory) == 6
    assert memory is dut.GetInternalSignal(name, is_array=True)
    words_name, tiny_name = find('words'), find('tiny')
    words = dut.GetInternalSignal(words_name, is_array=True)
    tiny = dut.GetInternalSignal(tiny_name, is_array=True)
    assert len(words) == len(tiny) == 3
    raw = (ctypes.c_uint32 * 18).from_address(dut.xcfg.Address(name))
    for index, element in enumerate(memory):
        assert element.GetBackendKind() == xsp.XDataBackendKind_MemDirect
        assert element.IsArrayBound()
        element.AsImmWrite()
        value = (1 << 64) | (0x123456789ABCDEF + index)
        element.value = value
        # Prove writes target the actual array storage before an eval or DPI call.
        assert sum(raw[3 * index + w] << (32 * w) for w in range(3)) == value
        selected = dut.GetInternalSignal(name, index=index)
        assert selected.value == value
        assert selected.IsArrayBound()
        assert array_view.AtFlat(index).value == value
        assert selected is dut.GetInternalSignal(name, index=index)
        assert selected is not memory
        # With trace metadata, RTL-indexed names resolve to the same native leaf.
        alias = f'{name}[{index // 3 + 2}][{index % 3 - 1}]'
        if sys.argv[1] != 'none':
            assert alias in names
            named = dut.GetInternalSignal(alias)
            assert named.GetBackendKind() == xsp.XDataBackendKind_MemDirect
            assert named.value == value
        dut.row.value, dut.col.value = index // 3, index % 3
        dut.RefreshComb()
        assert dut.value.value == value
    for index in (-2, 6):
        try:
            dut.GetInternalSignal(name, index=index)
        except IndexError:
            pass
        else:
            raise AssertionError('invalid native index accepted')
    assert dut.GetInternalSignal(name + '.missing', index=0) is None
    for index in range(3):
        words[index].AsImmWrite()
        tiny[index].AsImmWrite()
        value = (1 << 63) | index
        words[index].value = value
        tiny[index].value = index & 1
        assert ctypes.c_uint64.from_address(dut.xcfg.Address(words_name) + 8 * index).value == value
        assert ctypes.c_uint8.from_address(dut.xcfg.Address(tiny_name) + index).value == index & 1
        dut.col.value = index
        dut.RefreshComb()
        assert dut.word.value == value and dut.flag.value == index & 1
finally:
    dut.Finish()
"""

CPP_INTERNAL_CHECK = """
#include "UT_InternalArrays.hpp"
#include <cassert>

int main() {
    UTInternalArrays dut;
    dut.row.AsImmWrite(); dut.col.AsImmWrite(); dut.Step(1);
    std::string name;
    for (const auto &candidate : dut.GetInternalSignalList()) {
        if (candidate.size() >= 4 && candidate.substr(candidate.size() - 4) == ".mem") { name = candidate; break; }
    }
    assert(!name.empty());
    auto memory = dut.GetInternalSignal(name, true);
    assert(memory.size() == 6);
    auto view = dut.xcfg->NewXDataArrayView(name);
    assert(view->Shape() == std::vector<int>({2, 3}));
    assert(view->Address(5) == dut.xcfg->Address(name) + 5 * 12);
    auto raw = reinterpret_cast<uint32_t *>(dut.xcfg->Address(name));
    XData *previous = nullptr;
    for (int i = 0; i < 6; ++i) {
        auto selected = dut.GetInternalSignal(name, i);
        assert(selected && selected != previous && selected == dut.GetInternalSignal(name, i));
        assert(selected->GetBackendKind() == XDataBackendKind::MemDirect);
        assert(selected->IsArrayBound());
        selected->AsImmWrite(); selected->Set(123 + i);
        assert(raw[i * 3] == uint32_t(123 + i));
        assert(memory[i]->U() == uint64_t(123 + i));
        dut.row.Set(i / 3); dut.col.Set(i % 3); dut.RefreshComb();
        assert(dut.value.U() == uint64_t(123 + i));
        previous = selected;
    }
    for (int index : {-2, 6}) {
        bool caught = false;
        try { dut.GetInternalSignal(name, index); } catch (const std::out_of_range &) { caught = true; }
        assert(caught);
    }
    assert(dut.GetInternalSignal(name + ".missing", 0) == nullptr);
}
"""


class UnpackedArrayTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root = Path(__file__).resolve().parents[2]
        cls.picker = os.environ.get("PICKER_BIN", str(cls.root / "build/bin/picker"))

    def run_command(self, args, *, cwd=None, timeout=600, env=None):
        result = subprocess.run(args, cwd=cwd, env=env, capture_output=True, text=True, timeout=timeout)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return result

    def test_native_layout_generator(self):
        # Compile the real address generator against synthetic model headers.
        # The Verilator fixture deliberately pads rows; GSIM needs no toolchain here.
        headers = {
            "verilator": {
                "layout_types.hpp": """#pragma once
#include <cstddef>
#include <cstdint>
using IData = uint32_t;
template<int N> struct VlWide { uint32_t words[N]; };
template<class T, int N> struct VlUnpacked {
    T elements[N];
    uint32_t padding;
    T &operator[](std::size_t index) { return elements[index]; }
};
""",
                "VLayout___024root.h": """#pragma once
#include "layout_types.hpp"
struct VLayout___024root {
    IData/*31:0*/ scalar;
    VlUnpacked<VlUnpacked<VlWide<3>/*64:0*/, 3>, 2> mem;
};
""",
                "VLayout.h": """#pragma once
#include "VLayout___024root.h"
struct VLayout { VLayout___024root storage; VLayout___024root *rootp = &storage; };
""",
            },
            "gsim": {
                "Layout.h": """#pragma once
#include <cstdint>
struct SLayout {
    uint32_t _var_start;
    uint32_t scalar; // width = 21
    uint32_t mem[2][3]; // width = 21
    uint32_t _var_end;
};
""",
            },
        }
        for simulator, files in headers.items():
            with self.subTest(simulator=simulator), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                for name, content in files.items():
                    (base / name).write_text(content)
                source = base / ("VLayout___024root.h" if simulator == "verilator" else "Layout.h")
                output = base / "mem_direct"
                self.run_command([
                    self.picker, "export", str(source), "--sname", "Layout",
                    "--sdir", str(self.root / "template/mem_direct"), "--tdir", str(output),
                    "--sim", simulator, "--autobuild", "false",
                ])
                executable = output / "export.bin"
                self.run_command(["c++", "-std=c++17", "-I", str(base),
                                  *map(str, sorted(output.glob("*.cpp"))), "-o", str(executable)])
                config = self.run_command([str(executable)]).stdout
                self.assertIn("array_dimensions: [2, 3]", config)
                self.assertIn("array_strides: [40, 12]" if simulator == "verilator" else
                              "array_strides: [12, 4]", config)
                self.assertIn("mem_bytes: 12" if simulator == "verilator" else "mem_bytes: 4", config)
                scalar = re.search(r"(?ms)^  - name: scalar\n(.*?)(?=^  - name:|\Z)", config)
                self.assertIsNotNone(scalar)
                self.assertNotIn("array_", scalar.group(1))

    def test_array_io(self):
        for access in ("dpi", "mem_direct"):
            with self.subTest(access=access), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                source = base / "arrays.sv"
                source.write_text(RTL)
                output = base / "Arrays"
                instances = 2 if access == "dpi" else 1
                prefixes = [f"Arrays_{i}_" for i in range(instances)] if instances > 1 else []
                self.run_command([
                    self.picker, "export", str(source), "--sname", f"Arrays,{instances}", "--tname", "Arrays",
                    "--sdir", str(self.root / "template"), "--tdir", str(output),
                    "--sim", "verilator", "--lang", "python", "--rw", access,
                    "--autobuild", "false", "-V", "-Wno-fatal",
                ])
                leaves = {}

                def collect(node, prefix=""):
                    if "_" in node:
                        leaves[prefix] = node
                    for name, child in node.items():
                        if isinstance(child, dict):
                            collect(child, prefix + "_" + name if prefix else name)

                collect(json.loads((output / "signals.json").read_text()))
                for prefix in prefixes or [""]:
                    self.assertEqual(leaves[prefix + "matrix"]["Unpacked"], [[-1, 0], [3, 2]])
                    self.assertEqual(leaves[prefix + "matrix_out"]["Unpacked"], [[-1, 0], [2, 3]])
                wrapper = (output / "Arrays_top.sv").read_text()
                self.assertNotRegex(wrapper, r"\bassign\b")
                # _picker_array_0 is a real RTL port; no extra element storage may appear.
                self.assertNotRegex(wrapper, r"logic[^;\n]*\b_picker_array_[1-9][0-9]*\s*;")
                if access == "mem_direct":
                    self.assertNotIn('export "DPI-C" function get_', wrapper)
                    self.assertNotIn("BindDPIPtr", (output / "python/dut.py").read_text())
                    self.assertNotIn("BindDPIArray", (output / "python/dut.py").read_text())
                    self.assertNotRegex((output / "dut_base.cpp").read_text(), r'pin_address_map\["_picker_array_[1-9]')
                else:
                    getters = re.findall(r'export "DPI-C" function get_(\w+)xx\w+;', wrapper)
                    setters = re.findall(r'export "DPI-C" function set_(\w+)xx\w+;', wrapper)
                    # 20 ports (including 14 arrays), 10 writable input ports, per instance.
                    self.assertEqual(len(getters), 20 * instances)
                    self.assertEqual(len(setters), 10 * instances)
                    self.assertNotRegex(wrapper, r'function (?:get|set)__picker_array_[1-9][0-9]*xx')
                    self.assertEqual(wrapper.count("input int index;"), 21 * instances)
                self.assertIn("BindArrayElement", (output / "python/dut.py").read_text())
                self.assertNotIn("xfunction<", (output / "dut_base.hpp").read_text())
                self.run_command(["make", "-C", str(output), "NPROC=4"])
                self.run_command([sys.executable, "-c", CHECK, *prefixes], cwd=base,
                                 env={**os.environ, "ARRAY_ACCESS": access})

    def test_cpp_array_io(self):
        for access in ("dpi", "mem_direct"):
            with self.subTest(access=access), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                source = base / "arrays.sv"
                source.write_text(RTL)
                output = base / "Arrays"
                self.run_command([
                    self.picker, "export", str(source), "--sname", "Arrays",
                    "--sdir", str(self.root / "template"), "--tdir", str(output),
                    "--sim", "verilator", "--lang", "cpp", "--rw", access,
                    "--autobuild", "false",
                ])
                (output / "cpp/example.cpp").write_text(CPP_CHECK.replace("EXPECTED_BACKEND", "MemDirect" if access == "mem_direct" else "DPI"))
                self.run_command(["make", "-C", str(output), "NPROC=4"])
                # The generic C++ example Makefile masks runtime failures, so run it explicitly.
                self.run_command([str(output / "UTArrays_example")], cwd=output)

    def test_java_array_io(self):
        if not shutil.which("javac") or not shutil.which("java"):
            self.skipTest("Java toolchain unavailable")
        location = subprocess.run([self.picker, "--show_xcom_lib_location_java"],
                                  capture_output=True, text=True, timeout=60)
        if location.returncode:
            self.skipTest("xcomm Java binding unavailable")
        jar = location.stdout.strip().splitlines()[-1]
        for access in ("dpi", "mem_direct"):
            with self.subTest(access=access), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                source = base / "arrays.sv"
                source.write_text(RTL)
                output = base / "Arrays"
                self.run_command([
                    self.picker, "export", str(source), "--sname", "Arrays",
                    "--sdir", str(self.root / "template"), "--tdir", str(output),
                    "--sim", "verilator", "--lang", "java", "--rw", access,
                    "--autobuild", "false",
                ])
                (output / "java/example.java").write_text(JAVA_CHECK)
                self.run_command(["make", "-C", str(output), "NPROC=4"])
                self.run_command(["java", "-ea", "-cp", os.pathsep.join([jar, str(output / "UT_Arrays-java.jar")]),
                                  "com.ut.example"], cwd=output)

    def test_internal_array_native_storage(self):
        for language, trace in (("python", None), ("python", "fst"), ("cpp", None)):
            with self.subTest(language=language, trace=trace), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                source = base / "internal.sv"
                source.write_text(INTERNAL_RTL)
                output = base / "InternalArrays"
                args = [self.picker, "export", str(source), "--sname", "InternalArrays",
                        "--sdir", str(self.root / "template"), "--tdir", str(output),
                        "--sim", "verilator", "--lang", language, "--rw", "mem_direct",
                        "--autobuild", "false"]
                if trace:
                    args.extend(["-w", str(base / ("trace." + trace))])
                self.run_command(args)
                self.assertNotIn("_picker_array_", (output / "InternalArrays_top.sv").read_text())
                if language == "python":
                    self.assertNotIn("BindDPIPtr", (output / "python/dut.py").read_text())
                else:
                    (output / "cpp/example.cpp").write_text(CPP_INTERNAL_CHECK)
                self.run_command(["make", "-C", str(output), "NPROC=4"])
                config = (output / "InternalArrays_offset.yaml").read_text()
                self.assertIn("array_dimensions: [2, 3]", config)
                self.assertIn("array_strides: [36, 12]", config)
                if language == "python":
                    self.run_command([sys.executable, "-c", INTERNAL_CHECK, trace or "none"], cwd=base)
                else:
                    self.run_command([str(output / "UTInternalArrays_example")], cwd=output)

    def test_unsupported_types(self):
        cases = (
            ("input logic [3:0] data []", "unsupported type"),
            ("input logic [3:0] data [$]", "unsupported type"),
            ("input logic [3:0] data [int]", "unsupported type"),
            ("input real data [2]", "unsupported type"),
            ("inout wire [3:0] data [1:0]", "unsupported inout unpacked array"),
        )
        for declaration, diagnostic in cases:
            with self.subTest(declaration=declaration), tempfile.TemporaryDirectory() as directory:
                base = Path(directory)
                source = base / "unsupported.sv"
                source.write_text(f"module Unsupported({declaration}); endmodule\n")
                result = subprocess.run([
                    self.picker, "export", str(source), "--sname", "Unsupported",
                    "--sdir", str(self.root / "template"), "--tdir", str(base / "out"),
                    "--sim", "verilator", "--lang", "python", "--autobuild", "false",
                ], capture_output=True, text=True, timeout=60)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(diagnostic, result.stderr)


if __name__ == "__main__":
    unittest.main()
