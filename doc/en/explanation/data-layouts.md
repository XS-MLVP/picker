# XData type layouts and explicit views

**Applies to this branch's native ABI 6 source and matching SDK.** `XData` retains its configured type layout. Use `ArrayView()` / `StructView()` to obtain a structural view explicitly. See [exporting projects](../guides/export.md) and [internal signals](../guides/internal-signals.md) for usage.

## Data, Array, Struct, and packed

`XData` represents one complete fixed-width value; ordinary bit indexing continues to select a bit of that value. `XArray` is an array container whose index selects an element of the current dimension. `XStruct` is a container of named fields.

Packed and unpacked describe RTL type composition. Both a packed array and a packed struct can be accessed as one complete `XData`, or through an explicitly requested structural view. Unpacked containers have independently stored fields or elements.

| RTL type | Default signal representation | Structural access |
| --- | --- | --- |
| Ordinary packed bit vector | `XData` | Existing whole-value and bit access |
| Packed array | Complete `XData` | Explicitly request an array view |
| Packed struct | Complete `XData` | Explicitly request a struct view |
| Unpacked array | `XArray` | Select elements using declared bounds |
| Unpacked struct | `XStruct` | Select members by name |

For example:

```systemverilog
logic [1:0][7:4] matrix;
logic [1:0][7:4] mixed [3:2];
```

`matrix` defaults to one complete 8-bit `XData`. `mixed` is an unpacked `XArray`; `mixed[3]` defaults to one complete 8-bit `XData`. Ordinary indexing selects its bits; use `ArrayView()` to select elements according to `[1:0][7:4]`. An explicit view's layout determines its array and field selections.

Ordinary one-dimensional vectors also retain available declaration bounds. Data
declared `[7:4]` still default to bit offsets, while explicit `ArrayView()` can use
RTL indices 7 through 4. A vector discovered through VPI retains its declaration
range and defaults to the `bits` kind.

## Three coordinate systems

| Coordinate system | Meaning | Requirement |
| --- | --- | --- |
| `XData` bit offset | Least significant bit is 0 | Preserve existing semantics |
| Explicit RTL array-view index | Declared left/right, including directions and negative indices | Preserve declarations and check bounds |
| Legacy native-array storage coordinate | Zero-based storage positions exposed by entry points such as `NewXArray()` | Preserve zero-based storage coordinates |

For `logic [7:4] word`, RTL index 7 and Data bit offset 3 select the same bit. `word[3]` selects bit offset 3, while `word.ArrayView()[7]` uses the RTL index.

## What is already implemented

Configuration generation and runtime views use the following type and storage information:

1. Slang already collects fixed aggregate types, fields, bounds, signedness, and four-state attributes.
2. The `.xscfg` `ports` metadata contains port `Layout` records; native signals reference the type table through `layout_id`.
3. `XSignalCFG.NewXView(name)` already resolves native type layouts, checks storage, and constructs views.
4. `XData.ArrayView(layout)` and `StructView(layout)` already interpret an existing packed value with an external layout while sharing its storage.
5. Packed field slices, independent unpacked field bindings, and compiler-verified native offsets and strides already exist.

`NewXData()` associates verified types with returned Data; port wrappers call `BindDataLayouts()` after creating, binding, and registering Data. `port_types` maps exact port names to IDs in the interned type table. Unpacked containers use that graph and the original leaves through `NewViewFromPort()`.

XPort is a group of named Data. `NewViewFromPort(group, name)` obtains the named RTL object's structure from configuration and looks up existing Data in the group. `BindDataLayouts(group)` associates types with those Data.

## Usage

This fragment assumes a DUT with the corresponding packed ports has been exported and initialized with a matching ABI 6 SDK:

```python
dut.matrix.value = 0xA5
matrix = dut.matrix.ArrayView()
packet = dut.packet.StructView()
```

The argument-free forms use the type associated with the Data. Manual Data can use `SetLayout(layout)` or the explicit-layout view overloads. `HasLayout()` tests whether a type is associated. View requests validate the source, type, and width.

## Delivering type information to Data

### Configuration supplies types

xcfg looks up, validates, and reuses type descriptions on demand. Metadata is saved when `.xscfg` is generated; the association is established when a runtime signal is created or bound. Instances of the same type share an immutable description. Verified storage bindings supply native bases, padding, offsets, and strides.

### Connect both construction paths

- **Ports:** the wrapper creates and binds DPI/native Data, then associates the configured type.
- **Internal signals:** associate the selected node's type when Data is created on demand through `xcfg.NewXData()` or a recursive view.

Associations use RTL logical names or explicit type references. Both DPI and mem_direct projects provide port type configuration.

### Fields, elements, reinitialization, and lifetime

Data selected through a field or array layout retain the corresponding child type. Copying Data handles type information and backends separately. `ReInit()` clears associated types and view caches. View access checks the lifetime of its source Data.

## Implementation scope

| Layer | Main locations | Change |
| --- | --- | --- |
| xcomm Data | `include/xspcomm/xdata.h`, `src/data/data.cpp` | Optional shared type association; initialization, copying, reinitialization, and invalidation rules |
| xcomm views | `src/data/view.cpp` | Reuse type descriptions, add explicit entry points using the Data's own layout, and propagate child types |
| xcomm configuration | `src/core/signal_cfg.cpp` | Reuse existing type resolution and associate types with new or existing Data while keeping lookups lazy |
| Language bindings | `swig/xcomm.i`, `swig/view_raw.i`, `swig/python/xcomm.py` | Consistent entry points across six languages; check nested packed-node conversion and handle lifetime |
| Picker generation | `src/codegen/array.cpp`, `names.cpp`, language generators and templates | Associate port types; remove packed `*_view` members and their dedicated naming and registration |
| Picker types | `src/parser/sv.cpp` | Consistent layout interpretation for explicit port/native views |
| Tests and documentation | Both repositories' unit, binding, and real RTL tests and bilingual manuals | Validate entry points and migration usage |

Shared type and view-cache members change `XData`'s native object layout. This branch uses **ABI 6** with SONAME `libxspcomm.so.6` and Python dependency `uc-xcomm>=1.6,<2`. Regenerate and compile DUTs with the matching SDK.

## Validation and performance measurements

The ABI 6 aggregate fixture was verified on Verilator 5.052 across all six languages
for DPI, mem_direct, and VPI, using Scala 2.13.16. Additional Python regressions
cover nonzero/ascending vector bounds, nested packed/unpacked elements, and
consistent explicit port/native views.

The microbenchmark compares ABI 5 and ABI 6 with the same `.xscfg` containing
**711,754 synthetic records sharing a 32-bit struct type**, taking the median of
five independent processes. Values are actually bound and read; later phases
retain 4,096 Data. Cold-cache measurements use `POSIX_FADV_DONTNEED` advice.

| Metric | ABI 5 baseline | ABI 6 type association |
| --- | --- | --- |
| Local `sizeof(XData)` | 584 bytes | 616 bytes |
| Warm initialization through first successful read | 0.375 ms | 0.522 ms |
| Advised-cold initialization through first successful read | 5.26 ms | 8.01 ms |
| Repeated creation and read of same-name Data | 1.13 µs/call | 1.21 µs/call |
| Repeated struct-field view and read on the same Data | 14.67 µs/call | 0.144 µs/call |
| Process RSS after reading and retaining 4,096 Data | 33.8 MiB | 34.0 MiB |
| Process RSS after also retaining their views and fields | 44.8 MiB | 42.8 MiB |

The baseline repeats the old JSON-layout view call; the new entry point uses
previously associated types and caches.

See the [export guide](../guides/export.md), [internal-signal guide](../guides/internal-signals.md), and [DUT API](../reference/dut-api.md) for usage. External consumers migrate from automatically generated `*_view` members to explicit entry points. The configuration database's `Metadata("ports")` entry point remains available.
