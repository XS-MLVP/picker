#!/usr/bin/env bash
set -euo pipefail

picker_wheel_dir=${PICKER_WHEEL_DIR:-dist}
xcomm_wheel_dir=${XCOMM_WHEEL_DIR:-dist}
verify_python=${VERIFY_PYTHON:-python3}
picker_spec=uc-picker
xcomm_spec=uc-xcomm
if [[ -n "${PICKER_VERSION_EXPECTED:-}" ]]; then
  picker_spec="uc-picker==${PICKER_VERSION_EXPECTED}"
fi
if [[ -n "${XCOMM_VERSION_EXPECTED:-}" ]]; then
  xcomm_spec="uc-xcomm==${XCOMM_VERSION_EXPECTED}"
fi

test_dir=$(mktemp -d /tmp/picker-wheel-test.XXXXXX)
trap 'rm -rf -- "${test_dir}"' EXIT
swig_wheel_dir=${SWIG_WHEEL_DIR:-${test_dir}/tools}
if [[ -z "${SWIG_WHEEL_DIR:-}" ]]; then
  "${verify_python}" -m pip download --only-binary=:all: --no-deps \
    --dest "${swig_wheel_dir}" 'swig==4.5.1'
fi
"${verify_python}" -m venv "${test_dir}/venv"
"${test_dir}/venv/bin/python" -m pip install --no-index \
  --find-links="${picker_wheel_dir}" --find-links="${xcomm_wheel_dir}" --find-links="${swig_wheel_dir}" \
  "${picker_spec}" "${xcomm_spec}"
"${test_dir}/venv/bin/python" -m pip check

export PATH="${test_dir}/venv/bin:${PATH}"
cd "${test_dir}"
"${test_dir}/venv/bin/python" - <<'PY'
import os
from pathlib import Path
import shutil
import subprocess
import sys
from importlib.metadata import version

import xspcomm

swig = shutil.which("swig")
assert swig and Path(swig).parent == Path(sys.executable).parent, swig
subprocess.run([swig, "-version"], check=True)
Path("CMakeLists.txt").write_text('cmake_minimum_required(VERSION 3.15)\nproject(InstalledSWIG NONE)\nfind_package(SWIG 4.5 REQUIRED)\n')
subprocess.run(["cmake", "-S", ".", "-B", "swig-check"], check=True)

picker_version = version("uc-picker")
xcomm_version = version("uc-xcomm")
if os.environ.get("PICKER_VERSION_EXPECTED"):
    assert picker_version == os.environ["PICKER_VERSION_EXPECTED"]
if os.environ.get("XCOMM_VERSION_EXPECTED"):
    assert xcomm_version == os.environ["XCOMM_VERSION_EXPECTED"]
assert xspcomm.version() == xcomm_version
signal = xspcomm.XData(8, xspcomm.XData.InOut)
signal.Set(0xA5)
assert (signal.U() & 0xFF) == 0xA5
clock = xspcomm.XClock(lambda _: 0)
clock.Step(2)
assert clock.clk == 2
assert xspcomm.abi_version() == 3
assert xspcomm.XTriggerEngine.CoverageVersion() == 5
assert xspcomm.XCoveragePoint().kind == xspcomm.XCoveragePointKind_Value
assert xspcomm.XCoverageBin().point_id == 0
engine = xspcomm.XTriggerEngine(clock)
assert not clock.IsControlled()
engine.AcquireClock()
try:
    try:
        clock.Step(1)
    except RuntimeError:
        pass
    else:
        raise AssertionError("manual stepping bypassed native engine ownership")
    assert clock.GetHalfTick() == 4
    assert engine.RunUntil(2).advanced_ticks == 2
    wide = xspcomm.XData(128, xspcomm.XData.InOut)
    wide.Set((1 << 100) | 0xA5)
    lhs = engine.ExprNewBitsSignal(wide, False)
    rhs = engine.ExprNewBitsConst(128, False, (7).to_bytes(16, "little"))
    total = engine.ExprNewBits(xspcomm.ExprOp_ADD, lhs, rhs, 128, False)
    engine.ExprNewConst(0)  # Collection must remove this unused node.
    remap = engine.CollectExpressions(xspcomm.XIntVector([total]))
    total = remap[total]
    assert total >= 0 and engine.ExpressionNodeCount() == 3
    assert int.from_bytes(engine.ExprEvalBits(total), "little") == (1 << 100) | 0xAC
finally:
    engine.ReleaseClock()
assert not clock.IsControlled()
clock.Step(1)

picker = os.path.join(os.path.dirname(sys.executable), "picker")
output = subprocess.check_output([picker, "--version"], text=True)
assert picker_version in output, output
subprocess.run([picker, "export", "-h"], check=True)
invalid = subprocess.run([picker, "--not-a-picker-option"], capture_output=True)
assert invalid.returncode != 0, "the Python entry point swallowed the native failure"
print("installed picker", picker_version, "with xspcomm", xcomm_version)
PY
