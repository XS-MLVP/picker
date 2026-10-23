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

## Add debugging features

- Use `-w trace.fst` to enable waveforms; see [record simulation results](simulation.md#recording).
- Use `--coverage` to collect simulator code coverage.
- Use `--vpi` or `--internal` to access internal signals; see [internal signals](internal-signals.md).
- Use `--lang` to change the language; see [language examples](languages.md).

See the [CLI reference](../reference/cli.md) for all options, defaults, and output directory rules.
