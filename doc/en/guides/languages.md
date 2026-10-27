# Use a DUT from different languages

`--lang` supports `python`, `cpp`, `java`, `scala`, `golang`, and `lua`. Picker generates the selected language's wrappers, build files, and port interfaces.

## Build and run an existing example

From the Picker source directory, build support for the languages you need. For example, Python and Java:

```sh
make build NPROC=4 BUILD_XSPCOMM_SWIG=python,java
make test_Adder EXAMPLE_LANG=java
```

The second command exports Adder, compiles it, and runs `example/Adder/example.java`. Set `EXAMPLE_LANG` to `python`, `cpp`, `scala`, `golang`, or `lua` to run another example. The example script regenerates `output/Adder`.

Java and Scala need a JDK; Scala also needs its toolchain. Go needs the Go compiler, and Lua needs an interpreter and development headers. Run `picker --check` to inspect resources for the selected language.

## Export your own design

For example, export a C++ model:

```sh
picker export example/Adder/Adder.v --sname Adder --lang cpp \
    --tdir build/AdderCpp --autobuild false
make -C build/AdderCpp -j4
```

Use the corresponding `example/Adder/example.*` to learn the generated interface, then substitute your design and test.

## Read and write signals

| Language | Write | Read |
| --- | --- | --- |
| Python | `dut.a.value = 42` | `dut.sum.value` |
| C++ | `dut.a = 42` | `dut.sum.U()` |
| Java | `dut.a.Set(42)` | `dut.sum.U()` |
| Scala | `dut.a.Set(42)` | `dut.sum.U()` |
| Go | `dut.A.Set(42)` | `dut.Sum.U()` |
| Lua | `dut.a:Set(42)` | `dut.sum:U()` |

Python integers support the full signal width. C++ `U()` / `S()` return the low 64 bits; use `GetBytes()` / `SetBytes()` for wide signals. Java and Scala provide large-integer wrappers. Go and Lua can use byte or string interfaces for wide values; consult the generated signatures and language examples.

When a port name conflicts with a method, the generator adjusts it according to the language's naming rules, for example by adding `pin_`. Go's exported names start with a capital letter.

## Distribute the generated model

A compiled DUT package includes language interfaces, model libraries, and runtime resources. The default `--cp_lib true` copies runtime libraries. Distribute the complete package and run a short test in the target environment to check imports, signal access, and simulation stepping.

Python extensions must match the target Python minor version, operating system, and architecture.
