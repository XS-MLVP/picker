#!/usr/bin/env bash
set -euo pipefail

picker_wheel_dir=${PICKER_WHEEL_DIR:-dist}
xcomm_wheel_dir=${XCOMM_WHEEL_DIR:-dist}
verify_python=${VERIFY_PYTHON:-python3}
picker_spec=picker
xcomm_spec=xspcomm
if [[ -n "${PICKER_VERSION_EXPECTED:-}" ]]; then
  picker_spec="picker==${PICKER_VERSION_EXPECTED}"
fi
if [[ -n "${XCOMM_VERSION_EXPECTED:-}" ]]; then
  xcomm_spec="xspcomm==${XCOMM_VERSION_EXPECTED}"
fi

test_dir=$(mktemp -d /tmp/picker-wheel-test.XXXXXX)
trap 'rm -rf -- "${test_dir}"' EXIT
"${verify_python}" -m venv "${test_dir}/venv"
"${test_dir}/venv/bin/python" -m pip install --no-index \
  --find-links="${picker_wheel_dir}" --find-links="${xcomm_wheel_dir}" \
  "${picker_spec}" "${xcomm_spec}"
"${test_dir}/venv/bin/python" -m pip check

cd "${test_dir}"
"${test_dir}/venv/bin/python" - <<'PY'
import os
import subprocess
import sys
from importlib.metadata import version

import xspcomm

picker_version = version("picker")
xcomm_version = version("xspcomm")
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
assert xspcomm.abi_version() == 2

picker = os.path.join(os.path.dirname(sys.executable), "picker")
output = subprocess.check_output([picker, "--version"], text=True)
assert picker_version in output, output
subprocess.run([picker, "export", "-h"], check=True)
invalid = subprocess.run([picker, "--not-a-picker-option"], capture_output=True)
assert invalid.returncode != 0, "the Python entry point swallowed the native failure"
print("installed picker", picker_version, "with xspcomm", xcomm_version)
PY
