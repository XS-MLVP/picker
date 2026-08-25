# Coverage example

This example shows the picker coverage flow with one Python testbench for both
VCS and Verilator.

The core lifecycle is:

```text
run some stimulus
SetCoverage("snapshot_name")
FlushCoverage()
PrintCoverage()
Finish()
```

The example uses `example/DualPortStackCb/dual_port_stack.v`, whose reset logic,
two ports, command branches, response paths, and stack state make the coverage
changes visible across multiple snapshots.

## API behavior

`SetCoverage(name)` selects the next coverage snapshot.

- VCS uses `name` as the testdata name in the VDB.
- Verilator has one cumulative `coverage.dat`; named snapshots are not supported,
  so picker prints a warning and keeps using the cumulative database.

`FlushCoverage()` finalizes the current coverage snapshot while preserving the
simulator instance and the accumulated coverage counters.

`GetCoverage()` reads the existing coverage database and returns a structured
`CoverageReport` without printing. The lazy `Coverage` attribute is shorthand
for the default query, so tools can use `report = dut.Coverage` without exposing
an eager property to DUT signal introspection. Use
`GetCoverage(kind=..., module=..., instance=..., test=...)` when filters are
needed. Picker returns the structured object; Toffee owns JSON persistence.

`PrintCoverage()` performs the same query, prints a human-readable summary, and
returns the same structured report for compatibility. VCS queries `line`,
`toggle`, `branch`, `condition`, and `fsm` by default; Verilator currently
queries `line`. This portable example explicitly requests `line`, which is
implemented by both providers. Toggle items identify a signal bit and
transition (`0 -> 1` or `1 -> 0`); branch items identify the branch object and
uncovered arm.

`CoverageReport.items` contains uncovered objects. `covered_items` contains the
covered objects requested by the provider detail mode. Items carry stable source
identity, file/module/instance location, coverage kind, hit count, and native
atom information. `PrintCoverage()` keeps its human-readable output focused on
the summary and uncovered objects.

For VCS, a query without `test` merges all usable testdata in the package VDB.
If one testdata entry cannot be loaded or merged, the provider adapter retains
the successfully loaded testdata, records the failed entry in the report
diagnostics, and Python emits a `RuntimeWarning`. The query fails only when no
usable testdata remains. VCS snapshot finalization and provider queries share a
`.vdb.lock` file to synchronize access to the native coverage database.

`Finish()` finalizes the last coverage snapshot, finalizes the simulator
instance, and releases its runtime resources. The public API is idempotent:
calling it twice must not execute simulator finish twice, and the Python process
remains valid after simulator finalization.

## What the example demonstrates

The script runs three cumulative phases:

```text
reset_only
port0_push_pop
both_ports
```

After each phase it calls:

```python
dut.SetCoverage(name)
dut.FlushCoverage()
dut.PrintCoverage(kind="line")
```

Because `ResetCoverage()` is intentionally not called, covered counts are
checked for every returned metric and must not decrease from reset-only
stimulus to one-port stimulus to both-port stimulus. Line coverage is also
required to increase at each phase.

The script also flushes `both_ports` twice, using
`dut.GetCoverage(kind="line")` for the second query. VCS replaces the same named
testdata; Verilator rewrites the same cumulative `.dat` file. In both cases the
reported line coverage summary must stay unchanged.

## Data formats

- VCS stores simulator-native coverage in a `<DUT>.vdb` database directory.
- Verilator stores simulator-native coverage in a cumulative `coverage.dat`
  Coverage-3 data file.
- Picker generates `coverage/source-map.tsv` as the source identity map between
  simulator-reported paths and stable logical source identities.
- The generated provider adapter emits an IPC JSON payload on stdout. xspcomm
  deserializes the payload into a typed `CoverageReport` containing metric
  summaries, uncovered and covered coverage atoms, source locations, provider
  metadata, and diagnostics. Provider serialization is implemented in
  `template/coverage/vcs/vcs_uncover.cpp` or
  `template/coverage/verilator/verilator_coverage.cpp`.
- The IPC payload is not the persistent report schema. Toffee consumes the
  `CoverageReport` and owns coverage report artifact generation and retention.

The verification-facing `CoverageReport` fields are:

```text
success, query, metrics, items, covered_items,
errors, design_revision, inventory_revision
```

Each metric contains `kind`, `covered`, `total`, `uncovered`, and `rate`. Each
coverage atom carries its kind, source location, module, instance, object, hit
count, and metric-specific detail. See `doc/coverage.zh.md` for the query and
report field tables.

## Run

From the picker repository root:

```bash
make test_vcs_Coverage
make test_Coverage
```

The generated DUT and coverage database are written to `output/Coverage` by
default. Override the output root when needed:

```bash
make test_vcs_Coverage EXAMPLE_OUT_ROOT=/tmp/picker-coverage
```

The release scripts can also be run directly:

```bash
bash example/Coverage/release-vcs.sh
bash example/Coverage/release-verilator.sh
```

A successful run ends with output similar to:

```text
Python is still alive after DUT destruction
Coverage database: ...
Coverage example passed
```
