#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd verible-verilog-syntax
require_cmd cmake
require_cmd python3

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

blue "[export-vpi-python] Exporting InternalSignals with --vpi"
rm -rf "${ROOT_DIR}/picker_out_internal"
"${PICKER_BIN}" export \
  "${ROOT_DIR}/example/InternalSignals/vpi.v" \
  --autobuild false \
  --sdir "${ROOT_DIR}/template" \
  --sname vpi \
  --tdir  "${ROOT_DIR}/picker_out_internal/InternalSignals" \
  --lang python \
  --vpi \
  --sim verilator

cp "${ROOT_DIR}/example/InternalSignals/example.py" "${ROOT_DIR}/picker_out_internal/InternalSignals/python/"

blue "[export-vpi-python] Building generated project"
make -C "${ROOT_DIR}/picker_out_internal/InternalSignals" EXAMPLE=ON -j"$(nproc)"

green "[export-vpi-python] OK"
