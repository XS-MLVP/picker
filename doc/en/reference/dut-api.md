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

While a test framework holds clock control, `Step()` reports an error. Use the framework's execution method during that period. Newly generated VCS packages finalize in a child process so the host can continue reporting after `Finish()`; rebuild existing packages to obtain this behavior. UVS shutdown may still terminate the process, so arrange its test reporting before `Finish()`.

## Waveforms

| Method | Behavior and return value |
| --- | --- |
| `SetWaveform(filename)` | Set the output path, using the format configured at export time |
| `GetWaveFormat()` | Return an extension such as `fst`, or an empty string when disabled |
| `PauseWaveformDump()` | Pause recording |
| `ResumeWaveformDump()` | Resume recording |
| `WaveformPaused()` | Return 1 when paused and 0 when recording |
| `FlushWaveform()` | Write the waveform buffer to the file |

See [save waveforms](../guides/simulation.md#recording) for export options and a complete script. Frequent flushing adds I/O and affects FST compression efficiency.

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
| `GetInternalSignal(name, index=-1, is_array=False, use_vpi=False)` | Return a signal object by name, or a list with `is_array=True` |
| `GetInternalSignalList(prefix="", deep=99, use_vpi=False)` | List accessible names, preferring native layout information and then VPI |
| `VPIInternalSignalList(prefix="", deep=99)` | Enumerate hierarchical names through VPI |

For native lookup, `prefix` is a substring filter. For VPI, `prefix` and `deep` select the starting hierarchy and search depth. `index` and `is_array` apply to native layout lookup.

Results are cached by `name`, and later calls reuse the first object. An ordinary failed lookup returns `None`; an explicit `use_vpi=True` lookup failure raises an assertion. See [access internal signals](../guides/internal-signals.md) for examples.

## Python asynchronous methods

| Method | Behavior |
| --- | --- |
| `await dut.AStep(n)` | Wait for n simulation cycles |
| `await dut.ACondition(fn)` | Check fn at cycle notifications and return when it becomes true |
| `await dut.RunStep(n)` | Advance n cycles, allowing other coroutines to run between cycles |

Run waiting coroutines alongside a coroutine that advances the simulation. `AStep()` and `ACondition()` use Python cycle notifications from the clock.

## Framework integration

`GetXPort()` returns the DUT's `XPort` collection and `GetXClock()` returns its `XClock`. Frameworks can use these objects to manage signals and clocks. Use them and internal signal objects while the DUT is alive.
