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

`--rw mem_direct` builds an address generator against the generated model header,
using C++17 for ordinary models. With `--timing`, Picker detects the model header's
C++20 coroutine requirements and uses `-std=c++20 -fcoroutines`.

### Unpacked arrays and Verilator versions

The **DPI and mem_direct** paths for fixed unpacked arrays are supported with
**Verilator 5.026 and newer**. The general build minimum for other features is
5.020.

| Array path | RTL regression coverage | Verified Verilator versions |
| --- | --- | --- |
| DPI ports | Python, C++, Java | 5.026, 5.048, 5.052 |
| mem_direct ports | Python, C++, Java | 5.026, 5.048, 5.052 |
| mem_direct internal arrays | Python, C++ | 5.026, 5.048, 5.052 |

Optional Go/Lua port-array RTL tests are enabled with `ARRAY_EXTRA_LANGUAGES=1`.
The Scala aggregate fixture verifies unpacked containers with Scala 2.13.16 and
Verilator 5.052.

All six language generators use the shared array binding implementation.
Public arrays use xcomm's recursive `XArray` API, backed by shared internal bindings.
Use Picker and xcomm as a matched
toolchain, and regenerate and compile DUTs and single-file `.xscfg` configuration after upgrades.

Native element addresses and strides are computed by the C++ compiler against
the generated model. The access
code and layout metadata must be regenerated and compiled together when changing
Verilator or model build options.

Generated helper formats can change independently of the storage layout. Picker
selects the trace parser from the actual generated format, tolerates whitespace
in constant array selections. DPI prefers the declared C symbol. See
[array port access](../guides/export.md#export-unpacked-array-ports)
and [internal native arrays](../guides/internal-signals.md#access-a-native-array)
for the different indexing APIs.

### Struct and packed-array port views

Packed/unpacked structs and multidimensional packed arrays are exported with
the recursive xcomm containers described in [aggregate export](../guides/export.md#structs-and-multidimensional-packed-arrays).
Their minimum Verilator version is **5.048**. Ordinary unpacked arrays above retain their
5.026 minimum.
Packed objects bind one complete `XData`; unpacked objects bind independent
fields or elements. All layouts are derived from RTL, and native member
addresses/strides come from the generated C++ model.

| Paths | RTL regression languages | Verified Verilator versions |
| --- | --- | --- |
| DPI and mem_direct | Python, C++, Java | 5.048, 5.052 |
| DPI and mem_direct | Go, Lua | 5.048, 5.052 |
| VPI packed aggregates and unpacked arrays | Python, C++, Java, Go, Lua | 5.048, 5.052 |
| VPI recursive unpacked structs | Python, C++, Java, Go, Lua | 5.052 |
| DPI, mem_direct, VPI aggregate fixture | Scala 2.13.16 | 5.052 |

The 5.026/5.048 entries are historical regressions of the existing array/aggregate
interfaces. ABI 6 type association and argument-free explicit views were tested
with Verilator 5.052.

The tests include nested structs, arrays of structs, mixed-direction bounds,
65-bit fields and original whole packed values. Python additionally covers
packed arrays of structs, multidimensional packed/unpacked composition,
multiple instances, enums and integer fields. VPI views use VPI handles and
Picker's RTL type layout. Unpacked-struct VPI views require Verilator 5.052 or
newer. The VPI aggregate-port fixture was verified in all six language bindings.
VPI registers packed type layouts from ports.

Use the matching xcomm SDK with native ABI 6 and rebuild generated DUTs.
DPI setters and mem_direct writes access the selected independent field or
array element directly.

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

Linux is the primary build environment, and macOS supports Verilator and source builds. Select a Picker wheel for the operating system and architecture. The accompanying `uc-xcomm` wheel also matches the Python minor version; for example, `cp312` means CPython 3.12.

Use generated models with the toolchain and runtime libraries used during export. Regenerate and recompile after replacing those dependencies. Before distributing a model, run the short test from the [adder tutorial](../tutorials/first-dut.md) in the target environment to check imports, wide signals, and simulation stepping.
