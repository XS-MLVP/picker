#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd verible-verilog-syntax
require_cmd cmake
require_cmd python3

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

blue "[export-python] Exporting Adder.v to Python project"
rm -rf "${ROOT_DIR}/picker_out_adder"
"${PICKER_BIN}" export \
  "${ROOT_DIR}/example/Adder/Adder.v" \
  --autobuild false \
  --sdir "${ROOT_DIR}/template" \
  --sname Adder \
  --tdir  "${ROOT_DIR}/picker_out_adder/Adder" \
  --lang python \
  --sim verilator

cp "${ROOT_DIR}/example/Adder/example.py" "${ROOT_DIR}/picker_out_adder/Adder/python/"

blue "[export-python] Building generated project"
make -C "${ROOT_DIR}/picker_out_adder/Adder" EXAMPLE=ON -j"$(nproc)"

green "[export-python] OK"
