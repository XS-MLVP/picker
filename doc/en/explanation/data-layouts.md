# XData type layouts and explicit views

**Applies to this branch's native ABI 6 source and matching SDK.** This page describes the implemented type association and explicit views. Older ABI 5 SDKs do not support argument-free `ArrayView()` / `StructView()`; rebuild matching SDKs and DUTs. See [exporting projects](../guides/export.md) and [internal signals](../guides/internal-signals.md) for usage. Language and simulator support remain limited to actual regression coverage.

## The problem

Previously, the generator retained a complete `XData` for a packed aggregate and automatically added a `*_view` member. A packed array `matrix` produced both `dut.matrix` and `dut.matrix_view`; a packed struct `packet` produced both `dut.packet` and `dut.packet_view`. They shared signal storage, but added public names and initialized objects.

Previously, the configuration database contained type layouts but `XData` had no association with them. Explicit `ArrayView(layout)` and `StructView(layout)` calls required an external layout. Users should not have to describe RTL types a second time.

The goal is to **associate the existing type layout with the actual `XData`, obtaining a structural view only when explicitly requested, without automatically adding packed `*_view` members**.

## Data, Array, Struct, and packed

`XData` represents one complete fixed-width value; ordinary bit indexing continues to select a bit of that value. `XArray` is an array container whose index selects an element of the current dimension. `XStruct` is a container of named fields.

Packed and unpacked describe RTL type composition; they are not synonyms for Array and Data. Both a packed array and a packed struct can be accessed as one complete `XData`, or structurally after an explicit request. Unpacked containers have independently stored fields or elements and cannot be treated as one contiguous packed value.

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

`matrix` defaults to one complete 8-bit `XData`. `mixed` is an unpacked `XArray`; `mixed[3]` defaults to one complete 8-bit `XData`, not an automatically expanded packed subarray. Ordinary bit indexing of that Data remains bit selection. Interpreting it as `[1:0][7:4]` requires an explicit array-view request.

Once inside an explicit view, its layout determines array and field selection. A packed/unpacked boundary must not discard type information or change ordinary indexing merely because of a boolean attribute. Nested packed structs must follow the explicit-interpretation principle too; it cannot apply only to packed arrays.

Ordinary one-dimensional vectors also retain available declaration bounds. Data
declared `[7:4]` still default to bit offsets, while explicit `ArrayView()` can use
RTL indices 7 through 4. VPI discovery retains a single vector range without
changing its default bits kind. Missing ranges explicitly reject argument-free
array interpretation instead of inventing indices from width.

## Keep three coordinate systems distinct

This change does not redefine existing Data bit indexing or the storage coordinates of legacy native-array entry points.

| Coordinate system | Meaning | Requirement |
| --- | --- | --- |
| `XData` bit offset | Least significant bit is 0 | Preserve existing semantics |
| Explicit RTL array-view index | Declared left/right, including directions and negative indices | Preserve declarations and check bounds |
| Legacy native-array storage coordinate | Zero-based storage positions exposed by entry points such as `NewXArray()` | Do not silently replace with RTL indices |

For `logic [7:4] word`, RTL index 7 and Data bit offset 3 select the same bit. Retaining a type layout does not reinterpret `word[3]` as RTL index 3. Array dimensions in a view and bit offsets in a complete value are separate contracts.

## What is already implemented

Configuration generation and runtime views are not being built from scratch:

1. Slang already collects fixed aggregate types, fields, bounds, signedness, and four-state attributes.
2. The `.xscfg` `ports` metadata contains port `Layout` records; native signals reference the type table through `layout_id`.
3. `XSignalCFG.NewXView(name)` already resolves native type layouts, checks storage, and constructs views.
4. `XData.ArrayView(layout)` and `StructView(layout)` already interpret an existing packed value with an external layout while sharing its storage.
5. Packed field slices, independent unpacked field bindings, and compiler-verified native offsets and strides already exist.

This change adds **the association between the type layout and the actual `XData`**. `NewXData()` associates verified types with returned Data; port wrappers call `BindPortLayouts()` after creating, binding, and registering the original Data. Exact port-name-to-ID `port_types` metadata references the same interned type table rather than feeding automatically constructed packed `*_view` members. Unpacked containers use that graph and the original leaves through `NewPortView()`.

## Usage

This fragment assumes a DUT with the corresponding packed ports has been exported and initialized with a matching ABI 6 SDK:

```python
dut.matrix.value = 0xA5
matrix = dut.matrix.ArrayView()
packet = dut.packet.StructView()
```

The argument-free forms use the type associated with the Data. Callers do not write layouts or depend on `dut.matrix_view`, `dut.packet_view`, or other automatically added public names. Manual Data can explicitly associate a type with `SetLayout(layout)`; `HasLayout()` tests its presence, while view requests still validate the source and type.

The existing overloads accepting `layout` can still serve manually constructed Data; removing generated members does not imply removing those overloads. A manually constructed `XData(width)` has no recoverable RTL fields or array declarations. Width alone must not be used to guess a layout. Argument-free requests must fail explicitly if layout information is missing or its kind does not match.

## Delivering type information to Data

### Configuration supplies types without creating all views

xcfg is responsible for looking up, validating, and reusing type descriptions. Metadata is saved when `.xscfg` is generated; the association is established when an actual runtime signal is created or bound. Constructing xcfg must not expand every signal, parse every type, or create every view.

Instances of the same type should share an immutable description. Do not duplicate complete JSON on each Data or array element, retain a Slang AST, register per-element addresses, or add configuration sidecars. Type descriptions express semantics. Verified storage bindings still supply native bases, padding, offsets, and strides; those must not be guessed from type widths.

### Connect both construction paths

- **Ports:** associate the existing configuration type with the Data already constructed and bound to DPI/native storage by the wrapper. Do not replace it with a newly constructed native Data, which would change the backend, leaf identity, or edge-commit behavior.
- **Internal signals:** associate the selected node's type when Data is created on demand through `xcfg.NewXData()` or a recursive view. Do not enumerate every internal signal during DUT initialization.

Use RTL logical names or explicit type references, not renamed language attributes. Type lookup must not require a nonzero mem_direct base: DPI projects also contain port type configuration.

### Fields, elements, reinitialization, and lifetime

The association cannot stop at the root. Data obtained by selecting a field or array element through a layout must receive the corresponding child type. An arbitrary bit slice must not inherit the entire parent struct/array layout; preserve a corresponding type only when its extent and identity are known to match.

Copying Data must treat type information and backends separately: sharing an immutable type does not mean sharing model storage. `ReInit()` or a width change must clear or revalidate invalid type information and view caches rather than applying an old layout to a new width.

A Data type reference must not point into a destroyed xcfg's temporary objects. Conversely, retaining a type or view must not extend the validity of DUT memory. Existing model and parent-Data lifetime checks remain necessary. Obtaining a view must not change write modes, add XPort entries or clock callbacks, or introduce per-field SV bridges.

## Implementation scope

| Layer | Main locations | Change |
| --- | --- | --- |
| xcomm Data | `include/xspcomm/xdata.h`, `src/data/data.cpp` | Optional shared type association; initialization, copying, reinitialization, and invalidation rules |
| xcomm views | `src/data/view.cpp` | Reuse type descriptions, add explicit entry points using the Data's own layout, and propagate child types |
| xcomm configuration | `src/core/signal_cfg.cpp` | Reuse existing type resolution and associate types with new or existing Data while keeping lookups lazy |
| Language bindings | `swig/xcomm.i`, `swig/view_raw.i`, `swig/python/xcomm.py` | Consistent entry points across six languages; check nested packed-node conversion and handle lifetime |
| Picker generation | `src/codegen/array.cpp`, `names.cpp`, language generators and templates | Associate port types; remove packed `*_view` members and their dedicated naming and registration |
| Picker types | `src/parser/sv.cpp` | Consistent layout interpretation for explicit port/native views without changing default packed Data semantics |
| Tests and documentation | Both repositories' unit, binding, and real RTL tests and bilingual manuals | Validate new entry points, migrate generated-member usage, and distinguish verified support from untested scope |

Shared type and on-demand view-cache members change `XData`'s native object layout, so this branch uses **ABI 6** with SONAME `libxspcomm.so.6`. The ABI 5 `v1.5.0` SDK is not interchangeable. Picker's Python dependency lower bound is `uc-xcomm>=1.6,<2`. Update the pinned source dependency and rebuild matching SDKs/DUTs; there is no global object-address registry or legacy-layout compatibility layer. The view cache observes its source Data without owning DUT memory.

## Required validation

- Argument-free views use the associated type; missing layouts on manual Data, kind mismatches, and width mismatches produce explicit errors.
- Packed values still default to Data; `*_view` members are no longer automatically exported or created; unpacked containers remain usable.
- Packed arrays and packed structs follow the same explicit-access principle, with correct child types on fields and array elements.
- The same RTL declaration has consistent dimensions and bounds in explicit port and native views, preserving negative indices, directions, field order, and type attributes.
- Whole values and views share the same signal. Field writes preserve adjacent bits; DPI/native write modes, output directions, four-state behavior, and lifetime checks do not regress.
- VPI paths with sufficient semantic type information follow the same contract. Missing information produces an explicit capability diagnostic, not a guessed layout.
- Initialization does not expand every internal signal or array element, and on-demand requests do not repeatedly parse the same type. Measure initialization through a successful first read, repeated queries, and memory rather than constructors alone.
- Check C++, Python, Java, Scala, Go, and Lua interfaces and handle ownership together. Mark languages or simulators not actually run as unverified.

### Validation and performance boundaries

The ABI 6 aggregate fixture was verified on Verilator 5.052 across all six languages
for DPI, mem_direct, and VPI, using Scala 2.13.16. Additional Python regressions
cover nonzero/ascending vector bounds, nested packed/unpacked elements, and
consistent explicit port/native views. VCS, UVS, and GSIM aggregate runtimes and
complete large-core exports are not validated by these results.

The microbenchmark compares ABI 5 and ABI 6 with the same `.xscfg` containing
**711,754 synthetic records sharing a 32-bit struct type**, taking the median of
five independent processes. Values are actually bound and read; later phases
retain 4,096 Data. This is not full-model performance or full materialization.
Cold-cache measurements use `POSIX_FADV_DONTNEED` advice, not a forced system-cache flush.

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
previously associated types and caches. This isolates removal of repeated parsing,
not general value-read acceleration. Type parsing is charged to the new first
read, which does cost more, as does each Data's fixed metadata. Initialization
does not expand all 711,754 signals or construct all views.

See the [export guide](../guides/export.md), [internal-signal guide](../guides/internal-signals.md), and [DUT API](../reference/dut-api.md) for usage. External consumers must migrate from automatically generated `*_view` members to explicit entry points. The configuration database's `Metadata("ports")` entry point remains available. Signals actually declared with `*_view` names in RTL still generate normally; only extra generator-added members are removed.

## Outside this change

Do not rewrite database compression, indexing, or model identity; add JSON/YAML sidecars; change verified native address export; replace legacy storage coordinates with RTL indices; or bundle ordinary bit-index changes, tuple indexing, or per-bit PinBind allocation optimizations into this work. Those require their own scope and validation, separate from associating types with Data.
