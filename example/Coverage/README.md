# Coverage example

This example runs one Python testbench against both VCS and Verilator coverage.
The DUT is `example/DualPortStackCb/dual_port_stack.v`.

## Flow

Each stimulus phase uses the same sequence:

```python
dut.SetCoverage(name)
assert dut.FlushCoverage() == 0
report = dut.PrintCoverage(kind="line")
```

`SetCoverage(name)` selects the next VCS testdata name. Verilator has no named
testdata and continues accumulating counters in one `coverage.dat` file.

`FlushCoverage()` commits the current counters to a queryable simulator
database without ending the DUT. `GetCoverage()` queries that database and
returns a typed `CoverageReport`; `PrintCoverage()` also prints its summary and
uncovered items.

The default query kind is `line` for both simulators. VCS additionally supports
explicit `toggle`, `branch`, `condition`, and `fsm` queries. Verilator currently
supports line queries only.

The script runs these cumulative phases:

```text
reset_only
port0_push_pop
both_ports
```

It verifies that the coverage denominator is stable, covered line count
increases with additional stimulus, and flushing the same snapshot twice does
not change the result.

## Data

VCS writes a simulator-native `<DUT>.vdb` directory. Verilator writes a
Coverage-3 text database whose first line is:

```text
# SystemC::Coverage-3
```

The generated `coverage/coverage` executable reads the native database and
writes JSON to standard output. xspcomm converts that JSON to `CoverageReport`.
The important report fields are:

```text
schema_version, success, simulator, query, metrics, items, errors
```

`metrics` contains `covered`, `total`, `uncovered`, and `rate`. `items` contains
only uncovered objects and records their coverage kind, simulator-provided file
location, module, instance, object, hit count, and detail.

The C++ model is defined in
`dependence/xcomm/include/xspcomm/xcoverage.h`. The JSON generators are
`template/coverage/vcs/vcs_uncover.cpp` and
`template/coverage/verilator/verilator_coverage.cpp`. See
`doc/coverage.zh.md` for the complete current workflow and field tables.

## Run

From the Picker repository root:

```bash
make test_vcs_Coverage
make test_Coverage
```

The generated DUT and coverage database are written to `output/Coverage` by
default. An alternate output root can be selected with:

```bash
make test_vcs_Coverage EXAMPLE_OUT_ROOT=/tmp/picker-coverage
```

The release scripts can also be run directly:

```bash
bash example/Coverage/release-vcs.sh
bash example/Coverage/release-verilator.sh
```

A successful run ends with:

```text
Python is still alive after DUT destruction
Coverage database: ...
Coverage example passed
```
