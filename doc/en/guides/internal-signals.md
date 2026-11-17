# Access internal signals

When debugging a circuit, you can read internal registers or change signals by hierarchical name. This page starts with the repository's `vpi` example, then describes other access methods.

## Export a model with VPI

Run from the repository root:

```sh
picker export example/InternalSignals/vpi.v --sname vpi --lang python \
    --sim verilator --vpi --tdir build/docs-internal/vpi --autobuild false
make -C build/docs-internal/vpi -j4
```

`--vpi` enables runtime signal lookup. This design has a clock named `clk` and an internal signal, `_v2_base`, that controls a counter's increment.

## Find and change a signal

Save this code as `build/docs-internal/inspect.py`:

<!-- runnable:python -->
```python
from vpi import DUTvpi

dut = DUTvpi()
try:
    dut.InitClock("clk")
    dut.Step(1)
    for name in dut.VPIInternalSignalList():
        print(name)
    increment = dut.GetInternalSignal("vpi._v2_base", use_vpi=True)
    assert increment is not None
    increment.AsImmWrite()
    increment.value = 9
    dut.Step(1)
    before = dut.v2.value
    dut.Step(1)
    assert dut.v2.value == (before + 10) & 0xFF
    print("v2 increment: 10")
finally:
    dut.Finish()
```

Run:

```sh
cd build/docs-internal
python3 inspect.py
```

`Step(1)` first runs circuit initialization, and `AsImmWrite()` makes internal signal writes immediate. The output lists accessible hierarchical names and should end with `v2 increment: 10`. The RTL adds `1 + _v2_base` each cycle, so writing 9 makes the counter advance by 10.

For your own design, query names with `VPIInternalSignalList()`, then pass the actual hierarchical path to `GetInternalSignal()`. Use `prefix` and `deep` to narrow the search.

## Choose an access method

| Method | Configuration | Use |
| --- | --- | --- |
| Export selected internal signals | `--internal config-file` | Generate accessors for a known set of signals alongside ports |
| VPI lookup | `--vpi` | Enumerate and access signals by hierarchical name during debugging |
| Native memory access | `--rw mem_direct` | Direct reads and writes for Verilator/GSIM using the model's layout information |

By default, `GetInternalSignal()` uses generated native layout information when available and otherwise queries VPI. `use_vpi=True` selects VPI explicitly. `example/CacheSignalCFG` contains a complete native-access project.

`GetInternalSignal(..., use_vpi=True)` returns ordinary VPI leaves as `XData`.
Automatic discovery treats a single packed bit-vector dimension (including a
one-bit vector) as one `XData`; an unpacked array of such vectors is an `XArray`
with `XData` elements. Python packed aggregates also default to `XData`; explicitly
use its `StructView()` / `ArrayView()`. Unpacked containers return `XStruct`/`XArray`.
All six languages can also explicitly use `GetInternalSignalView(name)` for
recursive aggregates. Picker supplies the port's RTL layout
for packed aggregates; unpacked arrays and struct members are resolved to
checked VPI handles. Verilator 5.048 supports packed aggregate and unpacked-array
views, but cannot provide the VPI member/type information needed for unpacked
structs; those requests fail explicitly. Unpacked-struct VPI views require
Verilator 5.052 or newer. Internal unpacked structs can be discovered where the
simulator exposes their full VPI type/member information, but the VPI path does not
currently register internal packed-struct layouts with XVPI, so those are rejected
with an explicit missing-layout error. No simulator C++ memory layout is guessed.

## RTL types joined to native storage

Verilator `--rw mem_direct` exports reuse the internal types from the same Slang
elaboration and verify them against generated C++ storage. Every DUT exposes
`GetInternalSignalView(name, use_vpi=False)`: it prefers native metadata, otherwise
uses compiled-in VPI; `use_vpi=True` forces VPI. Without either backend, the call
reports the required build options. Aggregate ports are no longer a prerequisite
for generating this API.

For example, given these internal declarations:

```systemverilog
typedef struct packed { logic [7:0] tag; logic [23:0] data; } packet_t;
packet_t packet;
logic [1:0][31:0] lanes;
logic [64:0] memory [3:2][-1:1];
```

Use the full names actually returned by `GetInternalSignalList()`:

```python
packet = dut.GetInternalSignalView("Top.packet").Struct()
packet.Data("tag").value = 0x12  # Only the high eight bits of the shared value.
lanes = dut.GetInternalSignalView("Top.lanes").Array()
lanes.Data(0).value = 0x12345678
memory = dut.GetInternalSignalView("Top.memory").Array()
memory.Array(3).Data(-1).value = (1 << 64) | 0x55
```

These arrays use **RTL coordinates**, and ordinary single-dimension bit vectors
are `XData` leaves. The existing `GetInternalSignal(..., is_array=True)` retains
zero-based **storage coordinates**. Dynamically queried native-view leaves default
to immediate writes without clock callbacks; call `Step()` or `RefreshComb()` to
evaluate the affected logic. Whole packed values remain accessible through the
existing `GetInternalSignal(name)` or the view's `PackedData()`.

Data returned by native `GetInternalSignal(name)` retain available types. For
example, obtain the packet Data and call `StructView()` without supplying its
layout again. The view shares that Data's backend and existing write mode; it does
not create another immediate-write native backend. Legacy query paths with
different default write modes do not become the same object. Missing or unverified
layouts cannot be guessed from widths. Data bit indexing remains LSB-zero; only
explicit array views use RTL indices.

Fixed unpacked structs and arrays of them are supported when every member can be
matched to a real generated C++ member. Address differences and `sizeof` determine
padding, sizes and strides. Packed fields share their complete parent value;
unpacked fields bind independent storage. Neither VPI nor waveform tracing is
required.

Each export delivers one `<module>.xscfg` binary database containing port metadata,
interned types, native storage, member bindings and derived signals. There are no
default `signals.json`, `native_layouts.json`, offset YAML or index sidecars.
Java/Scala include the same database in the JAR and extract one temporary resource
per class, rather than copying it for each DUT. Neither the Slang AST nor array
elements are serialized.

Build-time candidate types live in the same file. Native address generation removes
candidate records and retains types reachable from verified bindings or port type references.
Ordinary values and homogeneous arrays reuse offsets/strides; unpacked structs add
homogeneous field-group `bindings`, not per-element registrations.

The database uses shared path prefixes, compact integers, a built-in read-only
lookup index and independent compressed blocks. POSIX readers map the file and
decompress only requested blocks, with a 2 MiB decompressed-block cache. They do not
populate maps for every signal. Full name enumeration still scans data blocks;
`String()` is an explicit full-inspection operation. The file format has its own
version; the current native ABI is 6. Readers verify the model identifier, file bounds and
checksums of accessed blocks, rejecting another build's database. A first access
to an uncached block has decompression cost; not every lookup will be faster than
an already fully loaded map.

Explicit inspection prints JSON to standard output without creating more files:

```sh
picker config Top.xscfg
picker config Top.xscfg --section metadata --name ports
picker config Top.xscfg --section variables --name Top.packet
```

Python can also call `json.loads(dut.xcfg.Metadata("ports"))` to obtain the original
port JSON structure. Third-party tools that only accept `signals.json` must adapt
to this entry point; no compatibility copy is generated by default.

Not every RTL name has writable retained storage. Optimized or relocated objects
may remain unaligned; width, shape, type or member mismatches do not receive a
view. Dynamic arrays, unions, strings and unrecognized native representations are
not supported here. Trace can supplement names, but temporary values and general
expressions are not writable addresses. Unbindable trace entries are marked
`unavailable` rather than aborting the whole signal-table build. Use native views
only while their DUT is alive. RTL four-state metadata does not make two-state
simulator storage preserve X/Z.

## Access a native array

On Verilator, native array access uses the `--rw mem_direct` layout path and the
same 5.026-and-newer range as array ports; regression anchors are 5.026, 5.048,
and 5.052. See [version compatibility](../reference/compatibility.md#unpacked-arrays-and-verilator-versions).

For fixed-size arrays in models with native layout information, obtain the array once and access its elements:

```python
memory = dut.GetInternalSignal("Top.memory", is_array=True)
print(memory[0][0].value)  # Example: a two-dimensional array
memory[1][0].value = 42
```

Replace `Top.memory` with a name returned by `GetInternalSignalList()`. A writable internal array must be retained by Verilator, for example with `/*verilator public_flat_rw*/` on its declaration. Neither VPI nor SV bridge signals are needed. A write in `AsImmWrite()` mode changes the model's storage immediately; call `Step()` or `RefreshComb()` to evaluate logic that reads it.

`is_array=True` returns a recursive `XArray`, not a flat list. Its indices are
zero-based **storage coordinates**, not the RTL bounds used by exported port views.
For `logic [64:0] memory [2:3][-1:1]`, `memory.shape` is `(2, 3)` and
`memory[row][column]` corresponds to RTL `memory[row + 2][column - 1]`.
`Size()` is 2 at the root and 3 in each row. The single-element
`dut.GetInternalSignal(name, index=position)` API still accepts a checked flat
storage position, with the last dimension varying fastest.

The layout generator records the dimensions and `sizeof`-derived strides. A 65-bit Verilator element occupies 12 bytes, not 9. With trace metadata, RTL-indexed leaf names such as `Top.memory[2][-1]` may also be available; direct element selections bind their native offsets rather than an expression backend. Use the names actually listed by the model. Computed expressions are still read-only derived signals, not writable native storage.

xcomm consumes dimensions and strides through one shared internal native backend.
`dut.xcfg.NewXArray(name)` returns the same cached recursive view; `Array(index)`
selects a subarray and `Data(index)` selects a leaf. Leaves are created on demand.
`LeftBounds()/RightBounds()/Shape()` describe its storage coordinates, and
retaining `cfg = dut.xcfg.At(name)` lets Python safely read `cfg.array_strides`
from the configuration object's layout metadata.
Recursive and single-element queries share the backend without per-leaf callbacks.
The backend retains metadata, not the model allocation or DPI library.

Use internal signal objects while their DUT is alive, and query them again after creating a new model.
