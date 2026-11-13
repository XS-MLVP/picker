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

`--rw mem_direct` builds a small address generator against the generated model header. When
Verilator also receives `--timing`, that header requires C++20 coroutines; Picker detects it and
builds the generator with `-std=c++20 -fcoroutines`. Without `--timing`, the generator stays on
C++17, so older compilers keep working for non-timing models.

### Unpacked arrays and Verilator versions

The **DPI and mem_direct** paths for fixed unpacked arrays are supported with
**Verilator 5.026 and newer**. The general build minimum for other features is
5.020.

| Array path | RTL regression coverage | Verified Verilator versions |
| --- | --- | --- |
| DPI ports | Python, C++, Java | 5.026, 5.048, 5.052 |
| mem_direct ports | Python, C++, Java | 5.026, 5.048, 5.052 |
| mem_direct internal arrays | Python, C++ | 5.026, 5.048, 5.052 |

All six language generators use the shared array binding implementation.
Array binding uses xcomm's `XDataArray` API. Use Picker and xcomm as a matched
toolchain, and regenerate and compile DUTs and array layout YAML after upgrades.

Native element addresses and strides are computed by the C++ compiler against
the generated model; they are not hard-coded per Verilator version. The access
code and layout metadata must be regenerated and compiled together when changing
Verilator or model build options.

Generated helper formats can change independently of the storage layout. Picker
selects the trace parser from the actual generated format, tolerates whitespace
in constant array selections, and falls back to legacy underscore-encoded DPI
symbol names only if the declared C name is absent. Unsupported trace formats
must be diagnosed, not treated as valid native layouts. See [array port access](../guides/export.md#export-unpacked-array-ports)
and [internal native arrays](../guides/internal-signals.md#access-a-native-array)
for the different indexing APIs.

### Struct and packed-array port views

Packed/unpacked structs and multidimensional packed arrays are exported with
the recursive xcomm containers described in [aggregate export](../guides/export.md#structs-and-multidimensional-packed-arrays).
Their minimum Verilator version is **5.048**; the generated CMake configuration
reports an error on older versions. Ordinary unpacked arrays above retain their
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

The tests include nested structs, arrays of structs, mixed-direction bounds,
65-bit fields and original whole packed values. Python additionally covers
packed arrays of structs, multidimensional packed/unpacked composition,
multiple instances, enums and integer fields. VPI views use VPI handles and
Picker's RTL type layout; they never infer simulator memory offsets. Verilator
5.048 supports the tested packed aggregates and unpacked arrays, but does not
expose enough VPI type/member information to traverse unpacked structs. Such a
request fails with the inaccessible member path; unpacked-struct VPI views
require 5.052 or newer. VPI results apply to the tested Python, C++, Java, Go
and Lua bindings on the tested aggregate-port fixture; Scala aggregate RTL runtime
behavior (DPI, mem_direct and VPI) has not been verified. Picker currently emits
packed type layouts for ports, not internal packed structs; an internal packed
struct request therefore fails instead of guessing offsets. These results do
not establish aggregate RTL support on VCS, UVS or GSIM.

Use the matching xcomm SDK with native ABI 4 and rebuild generated DUTs.
DPI setters and mem_direct writes access the selected independent field or
array element directly, without a whole unpacked-array copy.

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
