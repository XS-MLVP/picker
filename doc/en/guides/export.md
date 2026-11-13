# Export an RTL project

Exporting a design requires source files, a top module, a target language, and a simulator. Run the commands below from the Picker repository root. To use your own design, replace the file paths and module names.

## Export a single file

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/Adder --autobuild false
make -C build/Adder -j4
```

`--sname` selects the RTL top module. `--tdir` sets the output directory, and `--autobuild false` separates generation from compilation. With automatic building enabled by default, Picker compiles the project immediately after generation.

To use a different Python package name, such as `Arithmetic`, add `--tname Arithmetic` and change the output directory to `build/Arithmetic`. The generated class is then `DUTArithmetic`.

## Use a file list

For projects with multiple files, pass a file list through `--fs`. The repository's `example/MultiFileRTL/design.f` includes a header search path and several source files:

```text
+incdir+inc
inc/defs.vh
inc/outer.svh
leaf.v
top.sv
```

These paths are relative to the file list's directory. From the repository root, run:

```sh
picker export --fs example/MultiFileRTL/design.f \
    --sname MultiFileRTLTop --lang python --sim verilator \
    --tdir build/MultiFileRTLTop --autobuild false
make -C build/MultiFileRTLTop -j4
```

File lists support nested `-f` / `-F`, `+incdir+`, and `+define+` entries. Put the project's include directories and macro definitions in the list so parsing and simulator compilation use the same inputs.

## Pass compilation options

| Option | Stage | Common uses |
| --- | --- | --- |
| `-S` | slang parsing | Parser options and top-level parameters |
| `-V` | Simulator compilation | Simulator diagnostics or optimization |
| `-C` | C++ compilation | Compiler options |

Quote arguments containing spaces. For example, pass a warning option to Verilator:

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --tdir build/AdderDebug -V '-Wno-fatal' --autobuild false
```

## Export several modules together

`--sname` accepts module names and instance counts. If `rtl/a.v` defines `A` and `rtl/b.v` defines `B`, this command creates a combined module C with two A instances and three B instances:

```sh
picker export rtl/a.v,rtl/b.v --sname A,2,B,3 \
    --tname C --lang python --tdir build/C
```

Inspect the generated interface for instance and port names before writing the test.

## Export unpacked array ports

Fixed-size unpacked array ports are supported with the Verilator backend on
5.026 and newer; regression anchors are 5.026, 5.048, and 5.052. See the
[version compatibility notes](../reference/compatibility.md#unpacked-arrays-and-verilator-versions).

Fixed-size unpacked input/output arrays of scalar `bit`/`logic` or one-dimensional packed `bit`/`logic` vectors are supported. Generated interfaces preserve the array structure; each element is an ordinary `XData`:

```systemverilog
input logic [7:0] data [0:3];
input logic [7:0] matrix [-1:1][2:3];
```

For a Python DUT generated with these ports, use the original RTL indices:

```python
dut.data[0].value = 0x12
dut.matrix[-1][2].value = 0x34
for i in dut.data.indices:
    dut.data[i].value = i
dut.Step(1)
```

Python views provide `bounds` (for example, `((-1, 1), (2, 3))`), `shape`, `indices`, `len()`, and iteration over elements in declaration order. A negative index is an RTL index, not a position counted from the end. Each view level preserves the declared direction of its dimension; for `data [3:2]`, `indices` is `[3, 2]` and iteration starts at the left bound. Out-of-range indices raise `IndexError`; replace neither the view nor its elements, and assign to the element's `.value` instead. `dut["data"]` returns the same view and `dut["matrix[-1][2]"]` returns the same bound signal as `dut.matrix[-1][2]`.

| Language | Element access | Container |
| --- | --- | --- |
| Python | `dut.matrix[-1][2]` | `XArray`, with bound `XData` leaves |
| C++ | `dut.matrix->Array(-1)->Data(2)` | `shared_ptr<XArray>`, with shared leaves |
| Java | `dut.matrix.Array(-1).Data(2)` | `XArray` |
| Scala | `dut.matrix.Array(-1).Data(2)` | `XArray` |
| Go | `dut.Matrix.Array(-1).Data(2)` | `xspcomm.XArray` |
| Lua | `dut.matrix:Array(-1):Data(2)` | `XArray` |

C++ views throw `std::out_of_range` for an index outside the declared bounds.

Every language uses the same recursive `XArray` and checked RTL indices, not
language-specific maps. `Left()`, `Right()` and `Size()` describe the current
dimension; `LeftBounds()`, `RightBounds()` and `Shape()` describe consecutive
dimensions. `signals.json` records unpacked bounds as `Unpacked: [[left, right], ...]`
in declaration order. Multi-instance exports keep the usual module/instance prefix.
All six bindings use the same array-element binding path.

With Verilator `--rw mem_direct`, each array registers one base address and its
`sizeof`-derived byte strides. xcomm resolves element addresses when binding the
leaves, accounting for each RTL dimension's direction. No per-element address map,
SV storage, continuous-assignment bridges, or port DPI accessors are generated.
Internal arrays use the [native layout path](internal-signals.md#access-a-native-array), not the port interface or VPI.

With `--rw dpi`, each input array has **one indexed getter and one indexed setter**;
an output array has only the getter. These functions access the original SV array,
not bridge signals. Their internal flat index follows declaration order, with the
last dimension changing fastest; the generated SV code maps it back to RTL
subscripts. Both access modes use one shared internal C++ array backend. Each
leaf stores a backend reference and its DPI index or precomputed native address,
not a captured per-element read/write callback. Public indexing still uses the
RTL bounds.

With Toffee's `SignalArray` support, use `Bundle(matrix=dut.matrix)`
directly or bind the `Unpacked` metadata in `signals.json`. Toffee retains the
same leaves and original indices without rebinding their backend or registering
another `XPort`; snapshots retain bounds too. See the
[Toffee array guide](https://github.com/XS-MLVP/toffee/blob/main/docs/en/guides/data.md).

Views group references to the bound `XData` objects. A DPI backend read or write
uses the indexed accessor for one element; mem_direct accesses that element's
native storage directly. Generated port views create their leaves and register
them with `XPort` during initialization. Standalone `XArray.FromDPI` and
`XArray.FromNative` factories create and cache leaves on demand through the same
recursive interface. Shared binding and flat indexing are internal details.

This replaces the former `XDataArray` type and language-specific port containers;
there is no compatibility alias. Migrate full-coordinate/flat selections to one
`Array(index)` per dimension and `Data(index)` at the leaf (or Python indexing).
Regenerate DUTs with the matching Picker/xcomm SDK; the native ABI remains 4.

Dynamic arrays, queues, associative arrays, unions, and unpacked `inout` ports are not supported.

## Structs and multidimensional packed arrays

Picker generates recursive xcomm layouts from elaborated RTL types; no Toffee
layout declaration is needed. These aggregate port views require Verilator
**5.048 or newer**; generated builds reject older versions before compilation.

| RTL type | Bound values | Generated interface |
| --- | --- | --- |
| Packed struct | One complete `XData` | Original signal plus an `XStruct` field view |
| Multidimensional packed array | One complete `XData` | Original signal plus an `XArray` |
| Unpacked array | Independent element values | Array view over `XData` or nested containers |
| Unpacked struct | Independent field values | An `XStruct`, without a whole-value `XData` |

For a packed port `packet`, `dut.packet` remains the complete `XData` and
`dut.packet_view` maps its fields. A packed array `matrix` similarly gets
`dut.matrix_view`. If the suffix conflicts with another port, Picker appends
underscores until the view name is unique. Unpacked struct ports use their own
name directly. Structs and arrays compose recursively:

```python
dut.packed_in_view.Data("tag").value = 29
dut.packed_in_view.Data("valid").value = 1
dut.matrix_view[1][-2].value = 1
dut.record_in.entries[-1].header.Data("tag").value = 17
dut.record_in.entries[-1].payload[3].value = (1 << 64) + 0x1234
dut.Step(1)
```

Struct `Data(name)` returns the complete field `XData`; `Struct(name)` and
`Array(name)` select nested containers. Python also offers `view[name]` and
field attributes; a packed array field is an `XData` through these shortcuts,
and `Array(name)` explicitly requests its indexed view. Fields conflicting with
methods remain accessible with `view[name]`. Packed arrays retain each
dimension's RTL bounds and direction, with the leftmost declared element at the
most-significant end. Python `bounds`, `shape`, and `indices` follow unpacked
array conventions. Scalar leaves use `XData`'s usual width-zero representation.

Packed field writes merge into the parent and follow its existing write/edge
commit policy. Views do not add `XPort` registrations or simulator backends.
Unpacked fields bind independently; a homogeneous field within an unpacked
array shares one indexed DPI getter/setter pair, or one native base with
compiler-derived byte strides. Native access uses actual generated C++ members,
not assumed C++ struct packing. `signals.json` records the recursive type under
`Layout`, including fields, packed offsets, bounds, widths, signing and state kind.
DPI and mem_direct both access the selected independent field or array element
directly, without copying an entire unpacked array for a single-element write.

Run `make -C test export_aggregate_views` for the Python/C++/Java RTL regression
using `test/data/aggregate_views.sv`. Use the matching xcomm SDK containing
`XStruct`, `XArray` and native ABI 4, and rebuild generated DUTs with that SDK.

## Add debugging features

- Use `-w trace.fst` to enable waveforms; see [record simulation results](simulation.md#recording).
- Use `--coverage` to collect simulator code coverage.
- Use `--vpi` or `--internal` to access internal signals; see [internal signals](internal-signals.md).
- Use `--lang` to change the language; see [language examples](languages.md).

See the [CLI reference](../reference/cli.md) for all options, defaults, and output directory rules.
