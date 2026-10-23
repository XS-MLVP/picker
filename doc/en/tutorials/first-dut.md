# Test an adder with Python

This tutorial exports a 128-bit adder as a Python module. You will calculate `2 + 3 + 1`, then check carry and overflow with a repeatable test script.

Before starting, complete [installation](../guides/installation.md) and prepare Picker, Verilator, SWIG, and Python.

## 1. Get the adder

Clone the repository and enter its root directory. If you already have a checkout, enter that directory:

```sh
git clone https://github.com/XS-MLVP/picker.git
cd picker
```

Open `example/Adder/Adder.v` and find the `Adder` module. It has these ports:

| Port | Direction | Width | Meaning |
| --- | --- | --- | --- |
| `a`, `b` | Input | 128 | The two operands |
| `cin` | Input | 1 | Carry input |
| `sum` | Output | 128 | Low 128 bits of the result |
| `cout` | Output | 1 | Carry output |

The module computes `assign {cout, sum} = a + b + cin;`. It is a combinational circuit, so updating the inputs is enough to calculate a new result.

## 2. Export and compile

Run these commands from the repository root:

```sh
picker export example/Adder/Adder.v --sname Adder --lang python \
    --sim verilator --tdir build/docs-adder/Adder --autobuild false
make -C build/docs-adder/Adder -j4
```

The first command selects the `Adder` module and generates a project in `build/docs-adder/Adder`. The second compiles the RTL and Python interface. After compilation, the `Adder` directory is an importable Python package.

## 3. Calculate a sum

Save the following code as `build/docs-adder/check.py`:

<!-- runnable:python -->
```python
from Adder import DUTAdder

dut = DUTAdder()
dut.a.value = 2
dut.b.value = 3
dut.cin.value = 1
dut.Step(1)
print(f"sum={dut.sum.value}, cout={dut.cout.value}")
dut.Finish()
```

Run the script from the package's parent directory:

```sh
cd build/docs-adder
python3 check.py
```

The output should include:

```text
sum=6, cout=0
```

`DUTAdder()` creates a simulation model. Each port has an attribute with the same name; `.value` reads or writes its value. `Step(1)` submits the inputs to the simulator and updates the outputs. `Finish()` ends the simulation.

Change `cin.value` to `0` and run the script again. You should now get `sum=5, cout=0`.

## 4. Check carry and overflow

Replace `check.py` with the code below. It checks three input sets with assertions: an ordinary sum, the maximum value plus one, and two values with the high bit set.

<!-- runnable:python -->
```python
from Adder import DUTAdder

mask = (1 << 128) - 1
cases = [(2, 3, 1), (mask, 1, 0), (1 << 127, 1 << 127, 1)]
dut = DUTAdder()
try:
    for a, b, cin in cases:
        dut.a.value = a
        dut.b.value = b
        dut.cin.value = cin
        dut.Step(1)
        expected = a + b + cin
        assert dut.sum.value == (expected & mask)
        assert dut.cout.value == (expected >> 128)
        print(f"sum={dut.sum.value}, cout={dut.cout.value}")
    print("Adder: 3 cases passed")
finally:
    dut.Finish()
```

Run `python3 check.py` again. The output should include:

```text
sum=6, cout=0
sum=0, cout=1
sum=1, cout=1
Adder: 3 cases passed
```

Python integers hold the full result. `expected & mask` selects the low 128 bits for comparison with `sum`; `expected >> 128` selects the carry for comparison with `cout`. If a result differs, an assertion identifies the failing check.

You have exported a model, supplied inputs, advanced the simulation, and checked outputs. Next, [export your own RTL](../guides/export.md), or learn to [set up clocks and reset](../guides/simulation.md) for a sequential circuit.
