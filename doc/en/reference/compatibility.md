# Simulator and language support

## Simulators

`picker export --sim` accepts these values:

| Option | Environment | Notes |
| --- | --- | --- |
| `verilator` | Verilator and a C++ compiler | Default; used by the tutorials |
| `vcs` | VCS and its license; Verdi for FSDB | DPI/VPI, waveform, and code coverage integration |
| `gsim` | The GSIM toolchain | Export, simulation stepping, and native signal access integration |
| `uvs` | The UVS toolchain and license | Simulation stepping and USDB waveform integration |

Enable waveforms and checkpoints at export time:

| Feature | Export option | Support |
| --- | --- | --- |
| FST / VCD waveforms | `-w trace.fst` / `-w trace.vcd` | Verilator |
| FSDB waveforms | `-w trace.fsdb` | VCS + Verdi |
| USDB waveforms | `-w trace.usdb` | UVS |
| Code coverage | `--coverage` | Verilator and VCS; enabled metrics depend on build configuration |
| Checkpoints | `--checkpoints` | Verilator |
| Native memory access | `--rw mem_direct` | Verilator and GSIM |

## Languages

| `--lang` | Runtime or compiler |
| --- | --- |
| `python` | Python and matching development headers |
| `cpp` | A C++20 compiler |
| `java` | JDK |
| `scala` | JDK and Scala |
| `golang` | Go and a C++ compiler |
| `lua` | Lua and development headers |

Export requires installed binding resources for the selected language. See the [language guide](../guides/languages.md) for checks and example commands.

## Platforms and distribution

Linux is the primary build environment. macOS supports Verilator and source builds, with dynamic multi-instance use limited by platform loading behavior. Python wheels are specific to a Python minor version, operating system, and architecture; for example, `cp312` means CPython 3.12.

Use generated models with the toolchain and runtime libraries used during export. Regenerate and recompile after replacing those dependencies. Before distributing a model, run the short test from the [adder tutorial](../tutorials/first-dut.md) in the target environment to check imports, wide signals, and simulation stepping.
