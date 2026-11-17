# Python DUT API

The exported Python package provides a `DUT<module>` class. These signatures come from the Python template; see the [language guide](../guides/languages.md) for other wrappers and [simulator support](compatibility.md) for waveforms, coverage, and checkpoints.

## Create an object and access ports

```python
from Adder import DUTAdder

dut = DUTAdder()
dut.a.value = 42
print(dut.a.value)
print(dut["a"].value)
dut.Finish()
```

`DUT<module>(*args, **kwargs)` creates a model. Positional arguments are passed to the simulator. The keyword arguments `waveform_filename` and `coverage_filename` set output paths and require the corresponding features at export time.

`dut.pin` and `dut["pin"]` return the same port object. Write through `.value` or `Set(value)`. Reading `.value` returns an unsigned Python integer of the full signal width. The port attribute itself is read-only.

Packed structs and multidimensional packed arrays retain the whole `XData`
under the port name. Use `StructView()` / `ArrayView()` for explicit structural
access. Unpacked structs use `XStruct` directly; their array and struct fields
compose recursively. For example, `dut.record.entries[-1].flag`
is an ordinary `XData`. `Data(name)`, `Struct(name)` and `Array(name)` select
the desired field interface. See [aggregate export](../guides/export.md#structs-and-multidimensional-packed-arrays)
for the four storage models, original RTL indexing and examples.

## Clocks and execution

| Method | Behavior and return value |
| --- | --- |
| `InitClock(name)` | Register the named port as a clock pin; return `None` |
| `Step(i=1)` | Advance i complete cycles and refresh signals; return `None` |
| `StepRis(callback, args=(), kwargs={})` | Register a rising-edge callback as `callback(cycle, *args, **kwargs)` |
| `StepFal(callback, args=(), kwargs={})` | Register a falling-edge callback with the same arguments |
| `RefreshComb()` | Evaluate combinational logic while keeping the cycle count; inputs should already be submitted |
| `Finish()` | End the simulation and complete output handling; call after use |
| `AtClone()` | Reinitialize simulator state in a forked child process |

Callbacks run after evaluation and signal refresh at the corresponding edge. Drive the RTL reset port to apply reset; see [clock and reset setup](../guides/simulation.md).

New Python, C++, Java, Scala, Go, and Lua wrappers route `RefreshComb()` through
`XClock.RefreshComb()`. Request combinational evaluation through the framework
while it owns the clock. Regenerate and rebuild older packages. The Verilator
refresh path uses `eval_step()`; submit combinational inputs with `AsImmWrite()`.

Newly generated VCS packages finalize in a child process so the host can continue reporting after `Finish()`.

## Waveforms

| Method | Behavior and return value |
| --- | --- |
| `SetWaveform(filename)` | Set the output path, using the format configured at export time |
| `GetWaveFormat()` | Return an extension such as `fst` |
| `PauseWaveformDump()` | Pause recording |
| `ResumeWaveformDump()` | Resume recording |
| `WaveformPaused()` | Return 1 when paused and 0 when recording |
| `FlushWaveform()` | Write the waveform buffer to the file |

See [save waveforms](../guides/simulation.md#recording) for export options and a complete script.

## Code coverage and checkpoints

| Method | Behavior and return value |
| --- | --- |
| `SetCoverage(filename)` | Set the simulator coverage output location |
| `ResetCoverage()` | Reset coverage counters; currently implemented by the VCS path |
| `GetCovMetrics()` | Return the enabled metric mask; 0 means disabled |
| `CheckPoint(name)` | Save simulator state to a file; requires `--checkpoints`; Python returns `None` |
| `Restore(name)` | Restore simulator state from a checkpoint; Python returns `None` |

Bits 0–5 of `GetCovMetrics()` represent line, cond, fsm, toggle, branch, and assert.

## Internal signals

| Method | Behavior and return value |
| --- | --- |
| `GetInternalSignal(name, index=-1, is_array=False, use_vpi=False)` | Return a signal object; `is_array=True` returns a recursive `XArray` using zero-based storage coordinates |
| `GetInternalSignalView(name, use_vpi=False)` | Return an RTL-indexed recursive `XView`, preferring native metadata; `use_vpi=True` forces VPI |
| `dut.xcfg.Metadata("ports")` | Read port metadata from the `.xscfg` database as JSON |
| `GetInternalSignalList(prefix="", deep=99, use_vpi=False)` | List accessible names, preferring native layout information and then VPI |
| `VPIInternalSignalList(prefix="", deep=99)` | Enumerate hierarchical names through VPI |

For native lookup, `prefix` is a substring filter. For VPI, `prefix` and `deep` select the starting hierarchy and search depth.

Successful results are cached by name and lookup options; later calls with the same arguments reuse the first object. Native single-element `index` uses a flat storage position; VPI array selection uses that dimension's RTL index. See [access internal signals](../guides/internal-signals.md) for examples.

## Packed signal types and explicit views

These methods belong to `XData` objects returned as generated ports or internal
signals. Use the matching native ABI 6 SDK. Packed signals default to complete values.

| Method | Behavior |
| --- | --- |
| `data.HasLayout()` | Test whether a type is associated; conversion still checks width and source validity |
| `data.ArrayView()` | Obtain a shared-value `XArray` using the associated packed-array layout |
| `data.StructView()` | Obtain a shared-value `XStruct` using the associated packed-struct layout |
| `data.SetLayout(layout_json)` | Explicit type association for manual Data; xcfg already associates generated aggregate ports and available native types |
| `data.ArrayView(layout_json)` / `data.StructView(layout_json)` | Supply a layout explicitly |

Argument-free views and selected leaves are cached on demand. Default Python field access and packed
struct/array elements in unpacked arrays return Data; explicitly convert for
structural access. Ordinary Data bit indexing remains LSB-zero, while array views
use declared RTL indices. `ReInit()` clears types and caches.
See [type layouts and explicit views](../explanation/data-layouts.md).

## Python asynchronous methods

| Method | Behavior |
| --- | --- |
| `await dut.AStep(n)` | Wait for n simulation cycles |
| `await dut.ACondition(fn)` | Check fn at cycle notifications and return when it becomes true |
| `await dut.RunStep(n)` | Advance n cycles, allowing other coroutines to run between cycles |

Run waiting coroutines alongside a coroutine that advances the simulation. `AStep()` and `ACondition()` use Python cycle notifications from the clock.

## Framework integration

`GetXPort()` returns the DUT's `XPort` collection and `GetXClock()` returns its `XClock`. Frameworks can use these objects to manage signals and clocks. Use them and internal signal objects while the DUT is alive.
