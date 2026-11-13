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
For recursive aggregate ports, use `GetInternalSignalView(name)` in C++, Java,
Go, Lua and Scala; Python also returns `XStruct`/`XArray` from
`GetInternalSignal(name, use_vpi=True)`. Picker supplies the port's RTL layout
for packed aggregates; unpacked arrays and struct members are resolved to
checked VPI handles. Verilator 5.048 supports packed aggregate and unpacked-array
views, but cannot provide the VPI member/type information needed for unpacked
structs; those requests fail explicitly. Unpacked-struct VPI views require
Verilator 5.052 or newer. Internal unpacked structs can be discovered where the
simulator exposes their full VPI type/member information, but Picker does not
currently emit packed-struct layouts for internal signals, so those are rejected
with an explicit missing-layout error. No simulator C++ memory layout is guessed.

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
