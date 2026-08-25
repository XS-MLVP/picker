try:
    from UT_dual_port_stack import *
except ImportError:
    try:
        from dual_port_stack import *
    except ImportError:
        from __init__ import *


PUSH = 0
POP = 1
QUERY_KIND = "line"


def initialize_inputs(dut):
    dut.in0_valid.value = 0
    dut.in0_cmd.value = PUSH
    dut.in0_data.value = 0
    dut.out0_ready.value = 0
    dut.in1_valid.value = 0
    dut.in1_cmd.value = PUSH
    dut.in1_data.value = 0
    dut.out1_ready.value = 0


def transact(dut, port, command, data=0):
    in_valid = getattr(dut, f"in{port}_valid")
    in_ready = getattr(dut, f"in{port}_ready")
    in_cmd = getattr(dut, f"in{port}_cmd")
    in_data = getattr(dut, f"in{port}_data")
    out_valid = getattr(dut, f"out{port}_valid")
    out_ready = getattr(dut, f"out{port}_ready")

    in_cmd.value = command
    in_data.value = data
    in_valid.value = 1
    for _ in range(20):
        if in_ready.value:
            dut.Step(2)
            break
        dut.Step(1)
    else:
        raise RuntimeError(f"port {port} request did not become ready")
    in_valid.value = 0

    out_ready.value = 1
    for _ in range(20):
        if out_valid.value:
            dut.Step(2)
            break
        dut.Step(1)
    else:
        raise RuntimeError(f"port {port} response did not become valid")
    out_ready.value = 0


def coverage_metrics(report, name):
    metrics = []
    for metric in report.metrics:
        metrics.append(metric)
        assert metric.total > 0, f"{name}: coverage total must be positive"
        assert metric.covered + metric.uncovered == metric.total, (
            f"{name}: covered + uncovered must equal total"
        )
    assert metrics, f"{name}: no coverage metrics were returned"
    return metrics


def assert_cumulative_coverage(previous, current, previous_name, current_name):
    assert len(previous) == len(current)
    for before, after in zip(previous, current):
        assert before.kind == after.kind
        assert before.total == after.total, "coverage total changed"
        assert before.covered <= after.covered, (
            f"covered count decreased from {previous_name} to {current_name}"
        )


def assert_same_coverage(expected, actual):
    assert len(expected) == len(actual)
    for before, after in zip(expected, actual):
        assert before.kind == after.kind
        assert before.covered == after.covered
        assert before.total == after.total


def line_metric(report, name):
    metric = report.metric("line")
    assert metric is not None, f"{name}: line metric is missing"
    assert metric.covered + metric.uncovered == metric.total, (
        f"{name}: covered + uncovered must equal total"
    )
    return metric


def coverage_snapshot(dut, name):
    print()
    print(f"=== {name} ===")

    # SetCoverage() selects the next coverage snapshot.
    # VCS uses name as testdata. Verilator has no named testdata, so picker
    # warns and keeps writing its cumulative coverage.dat.
    dut.SetCoverage(name)

    # FlushCoverage() makes the current simulator coverage database queryable
    # without destroying the DUT and without resetting counters.
    assert dut.FlushCoverage() == 0

    # Query line coverage explicitly so the same example works with both VCS
    # and Verilator. VCS supports additional coverage kinds.
    #
    # PrintCoverage() prints a summary and returns a structured CoverageReport.
    report = dut.PrintCoverage(kind=QUERY_KIND)
    coverage_metrics(report, name)
    return report


def run_reset_only(dut):
    dut.rst.value = 1
    dut.Step(2)
    dut.rst.value = 0


def run_port_push_pop(dut, port, data):
    transact(dut, port=port, command=PUSH, data=data)
    transact(dut, port=port, command=POP)


def main():
    dut = DUTdual_port_stack()
    dut.InitClock("clk")
    initialize_inputs(dut)

    print(f"Coverage database: {dut.GetCoveragePath()}")

    run_reset_only(dut)
    reset_report = coverage_snapshot(dut, "reset_only")

    run_port_push_pop(dut, port=0, data=0x12)
    port0_report = coverage_snapshot(dut, "port0_push_pop")

    run_port_push_pop(dut, port=1, data=0x34)
    both_ports_report = coverage_snapshot(dut, "both_ports")

    # This example never calls ResetCoverage(), so snapshots are cumulative.
    assert_cumulative_coverage(
        reset_report.metrics, port0_report.metrics, "reset_only", "port0_push_pop"
    )
    assert_cumulative_coverage(
        port0_report.metrics, both_ports_report.metrics, "port0_push_pop", "both_ports"
    )
    reset_line = line_metric(reset_report, "reset_only")
    port0_line = line_metric(port0_report, "port0_push_pop")
    both_ports_line = line_metric(both_ports_report, "both_ports")
    assert reset_line.total == port0_line.total == both_ports_line.total
    assert reset_line.covered < port0_line.covered < both_ports_line.covered

    # Reusing the same snapshot name is allowed. VCS replaces that testdata;
    # Verilator rewrites the same cumulative coverage.dat. Coverage returns the
    # structured report without printing, which is convenient for other tools.
    dut.SetCoverage("both_ports")
    assert dut.FlushCoverage() == 0
    repeated_report = dut.GetCoverage(kind=QUERY_KIND)
    assert_same_coverage(both_ports_report.metrics, repeated_report.metrics)

    print()
    print("=== Finish ===")
    coverage_path = dut.GetCoveragePath()
    assert dut.Finish() == 0
    assert dut.Finish() == 0
    print("Python is still alive after DUT destruction")
    print(f"Coverage database: {coverage_path}")
    print("Coverage example passed")


if __name__ == "__main__":
    main()
