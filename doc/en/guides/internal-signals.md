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

## Access a native array

For models with native layout information, obtain the array once and access its elements:

```python
memory = dut.GetInternalSignal("Top.memory", is_array=True)
print(memory[0].value)
memory[1].value = 42
```

Replace `Top.memory` with the actual name. Results are cached by name, so keep the array lookup mode consistent for each name. The native access path provides `index` and `is_array`. Use internal signal objects while their DUT is alive, and query them again after creating a new model.
