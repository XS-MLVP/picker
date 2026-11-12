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
| Python | `dut.matrix[-1][2]` | Bounds-aware read-only view |
| C++ | `dut.matrix[-1][2]` | Bounds-checked view returning `XData&` |
| Java | `dut.matrix.get(-1).get(2)` | Read-only nested `Map` |
| Scala | `dut.matrix(-1)(2)` | Immutable nested `Map` |
| Go | `dut.Matrix[-1][2]` | Nested `map[int]` |
| Lua | `dut.matrix[-1][2]` | Nested table |

C++ views throw `std::out_of_range` for an index outside the declared bounds.

Every language uses the original RTL indices. The Java, Scala, Go, and Lua map containers retain their language's missing-key behavior. `signals.json` records unpacked bounds as `Unpacked: [[left, right], ...]` in declaration order. Multi-instance exports keep the usual module/instance prefix. All six bindings use the same array-element binding path, so element access follows the same backend rules and RTL indexing in every language.

With Verilator `--rw mem_direct`, each array registers one base address and its
`sizeof`-derived byte strides. xcomm resolves element addresses when binding the
leaves, accounting for each RTL dimension's direction. No per-element address map,
SV storage, continuous-assignment bridges, or port DPI accessors are generated.
Internal arrays use the [native layout path](internal-signals.md#access-a-native-array), not the port interface or VPI.

With `--rw dpi`, each input array has **one indexed getter and one indexed setter**;
an output array has only the getter. These functions access the original SV array,
not bridge signals. Their internal flat index follows declaration order, with the
last dimension changing fastest; the generated SV code maps it back to RTL
subscripts. Both access modes use xcomm's shared C++ `XDataArray` backend. Each
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
them with `XPort` during initialization. The C++ `XDataArray` API creates and
caches leaves on demand with `At(indices)`/`AtFlat(index)`.

The following are not supported: dynamic arrays, queues, associative arrays, unpacked `inout` ports, and element types other than the scalar/packed types above.

## Add debugging features

- Use `-w trace.fst` to enable waveforms; see [record simulation results](simulation.md#recording).
- Use `--coverage` to collect simulator code coverage.
- Use `--vpi` or `--internal` to access internal signals; see [internal signals](internal-signals.md).
- Use `--lang` to change the language; see [language examples](languages.md).

See the [CLI reference](../reference/cli.md) for all options, defaults, and output directory rules.
