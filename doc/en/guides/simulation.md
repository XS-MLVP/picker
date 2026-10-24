# Drive a simulation

This page uses the generated Python DUT interface to set up clocks, apply reset, sample signals, and record results. For a complete model creation example, see the [adder tutorial](../tutorials/first-dut.md).

## Set up a clock and reset

For a DUT with `clock` and active-high `reset` ports, run this after creating the object:

```python
dut.InitClock("clock")
dut.reset.value = 1
dut.Step(2)
dut.reset.value = 0
dut.Step(1)
```

`InitClock()` registers the clock pin. Each `Step(n)` then advances n complete cycles. This code holds reset for two cycles, releases it, and advances one more cycle. Set the port names, reset polarity, and duration to match your RTL protocol.

During a test, write inputs, call `Step()`, then read outputs. For example:

```python
dut.valid.value = 1
dut.data.value = 42
dut.Step(1)
print(dut.ready.value)
```

This fragment assumes the DUT has `valid`, `data`, and `ready` ports.

## Sample at an edge

Register a rising-edge callback with `StepRis()` or a falling-edge callback with `StepFal()`. This example reads `ready` after each rising edge:

```python
def observe(cycle):
    print(cycle, dut.ready.value)

dut.StepRis(observe)
dut.Step(3)
```

The first callback argument is the cycle count. Put sampling, logging, or assertions in the callback, and advance the simulation with `Step()` in the main test flow.

## Evaluate combinational logic

Set inputs that need immediate writes to `AsImmWrite()`, then call `RefreshComb()` after assigning them:

```python
dut.a.AsImmWrite()
dut.b.AsImmWrite()
dut.cin.AsImmWrite()
dut.a.value = 2
dut.b.value = 3
dut.cin.value = 1
dut.RefreshComb()
assert dut.sum.value == 6
```

This fragment uses the tutorial's Adder. Inputs are submitted immediately and combinational logic is evaluated while the cycle count stays unchanged.

<a id="recording"></a>

## Save waveforms and code coverage

Enable recording when exporting the model:

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/docs-trace/Adder --autobuild false \
    -w trace.fst --coverage
make -C build/docs-trace/Adder -j4
```

Save the following as `build/docs-trace/record.py`:

<!-- runnable:python -->
```python
from Adder import DUTAdder

dut = DUTAdder(waveform_filename="trace.fst", coverage_filename="coverage.dat")
try:
    dut.b.value = 1
    dut.cin.value = 0
    for value in range(4):
        dut.a.value = value
        dut.Step(1)
        assert dut.sum.value == value + 1
finally:
    dut.Finish()
```

Run the script and inspect the output files:

```sh
cd build/docs-trace
python3 record.py
ls -lh trace.fst coverage.dat
```

Open `trace.fst` in a waveform viewer such as GTKWave. You should see `a` change from 0 to 3 and `sum` from 1 to 4. Process the code coverage file with the simulator's coverage tools.

For a longer simulation, call `PauseWaveformDump()` to pause recording and `ResumeWaveformDump()` before the interval of interest. Call `FlushWaveform()` to inspect the file before the simulation ends. `Finish()` ends the simulation and completes output handling.

## Save and restore checkpoints

For a Verilator project, add `--checkpoints` during export, then call:

```python
dut.CheckPoint("state.bin")
dut.Step(10)
dut.Restore("state.bin")
```

A checkpoint saves simulator state. Save and restore the Python reference model, queues, and random generator state alongside it in your test program. See the [DUT API](../reference/dut-api.md) for return values and simulator support.
