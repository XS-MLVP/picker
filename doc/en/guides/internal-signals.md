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

Current VPI lookup does not provide array views: `is_array` and `index` apply only
to native layout lookup, even when the simulator exposes array handles. The planned
VPI array path requires **Verilator 5.048 or newer** and excludes 5.026; existing
VPI queries for individual signals are unchanged. See [VPI array compatibility](../reference/compatibility.md#vpi-array-versions)
for the distinction between simulator probes and current integration status.

## Access a native array

For models with native layout information, obtain the array once and access its elements:

```python
memory = dut.GetInternalSignal("Top.memory", is_array=True)
print(memory[0].value)
memory[1].value = 42
```

Replace `Top.memory` with a name returned by `GetInternalSignalList()`. A writable internal array must be retained by Verilator, for example with `/*verilator public_flat_rw*/` on its declaration. Neither VPI nor SV bridge signals are needed. A write in `AsImmWrite()` mode changes the model's storage immediately; call `Step()` or `RefreshComb()` to evaluate logic that reads it.

`is_array=True` returns a flat list in **native storage order**, with the last dimension varying fastest. Its indices are zero-based storage positions, not the RTL bounds used by exported port views. For `logic [64:0] memory [2:3][-1:1]`, the list has six elements and position `row * 3 + column` corresponds to RTL `memory[row + 2][column - 1]`. To bind only one element, use `dut.GetInternalSignal(name, index=position)`. Python and C++ check this flat index before binding and cache each index separately.

The layout generator records the dimensions and `sizeof`-derived strides; it checks that the array is contiguous before exposing the flat API. A 65-bit Verilator element occupies 12 bytes, not 9. With trace metadata, RTL-indexed leaf names such as `Top.memory[2][-1]` may also be available; direct element selections bind their native offsets rather than an expression backend. Use the names actually listed by the model. Computed expressions are still read-only derived signals, not writable native storage.

xcomm consumes the dimensions and strides through a shared native `XDataArray`
backend. `dut.xcfg.NewXDataArrayView(name)` returns a view with `Shape()`,
`Strides()`, `At(indices)` and `AtFlat(index)`; leaves are created on demand.
These internal view bounds are zero-based **storage coordinates**, not recovered
RTL bounds. Existing flat list/index queries use the same backend without per-leaf
callbacks. The backend retains metadata, not the model allocation or DPI library.

Use internal signal objects while their DUT is alive, and query them again after creating a new model.
