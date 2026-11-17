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
| `--rtl-param` | RTL elaboration | Design parameters; the value is pinned in the generated wrapper |
| `--rtl-define` | RTL and simulator | Preprocessor macros used by the RTL |
| `-S` | slang parsing | Parser options |
| `-V` | Simulator compilation | Simulator diagnostics or optimization |
| `-C` | C++ compilation | Compiler options |

Quote arguments containing spaces. For example, pass a warning option to Verilator:

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --tdir build/AdderDebug -V '-Wno-fatal' --autobuild false
```

Picker generates a `<module>_top.sv` wrapper and instantiates the design inside it, so
the simulator's top module is the wrapper. That is why design parameters have a
dedicated option: picker evaluates `--rtl-param` with the RTL frontend and writes the
resulting constants into the instantiation.

```sh
picker export t.sv --sname t --tdir out \
    --rtl-param A=8 --rtl-param C=2 --rtl-define DATA_WIDTH=16
```

```verilog
 t #(
    .A(8),
    .B(4),
    .C(2)
 ) t(
```

`--rtl-define` is forwarded to the RTL frontend as `-D` and to the simulator as
`+define+`. It takes precedence over the same macro in a filelist.

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

Python views provide `bounds` (for example, `((-1, 1), (2, 3))`), `shape`, `indices`, `len()`, and iteration over elements in declaration order. A negative index is an RTL index. Each view level preserves the declared direction of its dimension; for `data [3:2]`, `indices` is `[3, 2]` and iteration starts at the left bound. Out-of-range indices raise `IndexError`; assign values through the element's `.value`. `dut["data"]` returns the same view and `dut["matrix[-1][2]"]` returns the same bound signal as `dut.matrix[-1][2]`.

| Language | Element access | Container |
| --- | --- | --- |
| Python | `dut.matrix[-1][2]` | `XArray`, with bound `XData` leaves |
| C++ | `dut.matrix->Array(-1)->Data(2)` | `shared_ptr<XArray>`, with shared leaves |
| Java | `dut.matrix.Array(-1).Data(2)` | `XArray` |
| Scala | `dut.matrix.Array(-1).Data(2)` | `XArray` |
| Go | `dut.Matrix.Array(-1).Data(2)` | `xspcomm.XArray` |
| Lua | `dut.matrix:Array(-1):Data(2)` | `XArray` |

C++ views throw `std::out_of_range` for an index outside the declared bounds.

Every language uses the same recursive `XArray` and checked RTL indices.
`Left()`, `Right()` and `Size()` describe the current
dimension; `LeftBounds()`, `RightBounds()` and `Shape()` describe consecutive
dimensions. The database's `ports` metadata records unpacked bounds as `Unpacked: [[left, right], ...]`
in declaration order. Multi-instance exports keep the usual module/instance prefix.
All six bindings use the same array-element binding path.

With Verilator `--rw mem_direct`, each array registers one base address and its
`sizeof`-derived byte strides. xcomm resolves element addresses when binding the
leaves, accounting for each RTL dimension's direction.
Internal arrays use the [native layout path](internal-signals.md#access-a-native-array).

With `--rw dpi`, each input array has **one indexed getter and one indexed setter**;
an output array has only the getter. These functions access the original SV array.
Their internal flat index follows declaration order, with the
last dimension changing fastest; the generated SV code maps it back to RTL
subscripts. Both access modes use one shared internal C++ array backend. Each
leaf stores a backend reference and its DPI index or precomputed native address.
Public indexing uses the
RTL bounds.

With Toffee's `SignalArray` support, use `Bundle(matrix=dut.matrix)`
directly or adapt its metadata binding to the database's `ports` record. Toffee
retains the same leaves and original indices; snapshots retain bounds too. See the
[Toffee array guide](https://github.com/XS-MLVP/toffee/blob/main/docs/en/guides/data.md).

Views group references to the bound `XData` objects. A DPI backend read or write
uses the indexed accessor for one element; mem_direct accesses that element's
native storage directly. Generated port views create their leaves and register
them with `XPort` during initialization. Standalone `XArray.FromDPI` and
`XArray.FromNative` factories create and cache leaves on demand through the same
recursive interface. Shared binding and flat indexing are internal details.

This replaces the former `XDataArray` type and language-specific port containers.
Migrate full-coordinate/flat selections to one
`Array(index)` per dimension and `Data(index)` at the leaf (or Python indexing).
Regenerate DUTs with the matching Picker/xcomm SDK; the current native ABI is 6.

## Structs and multidimensional packed arrays

Picker generates recursive xcomm layouts from elaborated RTL types. These
aggregate port views require Verilator **5.048 or newer**.

| RTL type | Bound values | Generated interface |
| --- | --- | --- |
| Packed struct | One complete `XData` | Original signal with explicit `StructView()` |
| Multidimensional packed array | One complete `XData` | Original signal with explicit `ArrayView()` |
| Unpacked array | Independent element values | Array view over `XData` or nested containers |
| Unpacked struct | Independent field values | An `XStruct` |

Packed ports generate only their complete `XData` by default. Initialization
associates their types from the same `.xscfg`; explicitly call `StructView()` or
`ArrayView()`. Unpacked structs/arrays expose their own port containers.
Structs and arrays compose recursively:

```python
dut.packed_in.StructView().Data("tag").value = 29
dut.packed_in.StructView().Data("valid").value = 1
dut.matrix.ArrayView()[1][-2].value = 1
dut.record_in.entries[-1].header.StructView().Data("tag").value = 17
dut.record_in.entries[-1].payload[3].value = (1 << 64) + 0x1234
dut.Step(1)
```

Struct `Data(name)` returns the complete field `XData`; `Struct(name)` and
`Array(name)` select nested containers. Python also offers `view[name]` and
field attributes. Packed struct/array fields and packed elements of unpacked
arrays default to `XData`; explicitly use its `StructView()` / `ArrayView()`.
The container's `Struct(name)` and `Array(name)` are also explicit selections. Fields conflicting with
methods remain accessible with `view[name]`. Packed arrays retain each
dimension's RTL bounds and direction, with the leftmost declared element at the
most-significant end. Python `bounds`, `shape`, and `indices` follow unpacked
array conventions. Scalar leaves use `XData`'s usual width-zero representation.

Ordinary `XData` bit indexing remains LSB-zero; explicit array views use declared
RTL bounds. For `logic [1:0][7:4] mixed [3:2]`, `mixed[3]` is the complete 8-bit
Data and `mixed[3][1]` selects its bit offset 1. `mixed[3].ArrayView()[1][7]` is the
RTL array selection. See [XData type layouts and explicit views](../explanation/data-layouts.md).

Packed field writes merge into the parent and follow its existing write/edge
commit policy.
Unpacked fields bind independently; a homogeneous field within an unpacked
array shares one indexed DPI getter/setter pair, or one native base with
compiler-derived byte strides. Native access uses actual generated C++ members.
The database's `ports` record stores the recursive type under
`Layout`, including fields, packed offsets, bounds, widths, signing and state kind.
The same file's `port_types` maps exact logical port names to the interned type
table. xcfg associates types with existing Data. Types and on-demand views are
shared; field/element Data retain their child types.
DPI and mem_direct both access the selected independent field or array element
directly.

Run `make -C test export_aggregate_views` for the Python/C++/Java RTL regression
using `test/data/aggregate_views.sv`. Argument-free explicit views additionally have
six-language aggregate-fixture coverage on Verilator 5.052 for DPI, mem_direct,
and VPI (Scala 2.13.16). Use the matching xcomm SDK containing
argument-free explicit views and native ABI 6, and rebuild generated DUTs with that SDK.

## Generated member names

Leaf ports, unpacked containers and hierarchy subports share one member-name
allocator. Ordinary identifiers keep their spelling (Go capitalizes the first
letter). Wrapper members, methods and target-language keywords are protected;
conflicting ports use a `pin_` prefix, with a numeric suffix if that name is
already occupied. For example, with both `vpi_backend` and `pin_vpi_backend`
ports, Python exposes the former as `pin_vpi_backend_2` and keeps the latter's
original name.

Only wrapper attributes are renamed. RTL paths, `XPort` keys and the database's
port metadata keep their logical names. Use `dut["vpi_backend"]` or the language's
`XPort` lookup when consuming metadata. Verilator's native identifier escaping is separate from
these host-language names.

Every binding emits its `InOut` direction for packed and vector `inout` ports
(`IOType::InOut` in C++, `IOType_InOut` in Go, `XData.InOut` elsewhere). This
preserves direction metadata and makes the signal readable after the DUT drives it.

## Add debugging features

- Use `-w trace.fst` to enable waveforms; see [record simulation results](simulation.md#recording).
- Use `--coverage` to collect simulator code coverage.
- Use `--vpi` or `--internal` to access internal signals; see [internal signals](internal-signals.md).
- Use `--lang` to change the language; see [language examples](languages.md).

See the [CLI reference](../reference/cli.md) for all options, defaults, and output directory rules.
