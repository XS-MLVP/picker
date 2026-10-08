#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd cmake
require_cmd python3

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

blue "[export-python] Exporting Adder.v to Python project"
rm -rf "${ROOT_DIR}/picker_out/Adder"
"${PICKER_BIN}" export \
  "${ROOT_DIR}/example/Adder/Adder.v" \
  --autobuild false \
  --sdir "${ROOT_DIR}/template" \
  --sname Adder \
  --tdir  "${ROOT_DIR}/picker_out/Adder" \
  --lang python \
  --sim verilator

cp "${ROOT_DIR}/example/Adder/example.py" "${ROOT_DIR}/picker_out/Adder/python/"

blue "[export-python] Building generated project"
make -C "${ROOT_DIR}/picker_out/Adder" EXAMPLE=ON -j"$(nproc)"

blue "[export-python] Verifying generated signals against xcomm ABI 2"
(
  cd "${ROOT_DIR}/picker_out"
  python3 - <<'PY'
import Adder as generated

assert generated.xsp.abi_version() == 2
dut = generated.DUTAdder()
try:
    for name in ("a", "b", "cin", "sum", "cout"):
        signal = getattr(dut, name)
        assert isinstance(signal, generated.xsp.XData), name
        assert dut[name].CSelf() == signal.CSelf(), name
    try:
        dut.a = 0
    except AssertionError:
        pass
    else:
        raise AssertionError("assigning a signal must use its value property")

    mask = (1 << 128) - 1
    for a, b, cin in ((0, 0, 0), (mask, 1, 0), (1 << 127, 1 << 127, 1)):
        dut["a"].value, dut.b.value, dut.cin.value = a, b, cin
        dut.Step(1)
        expected = a + b + cin
        assert dut.sum.value == expected & mask
        assert dut.cout.value == expected >> 128
finally:
    dut.Finish()
PY
)

green "[export-python] OK"
