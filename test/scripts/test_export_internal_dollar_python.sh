#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd cmake
require_cmd grep
require_cmd python3
require_cmd tee

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
OUTPUT_DIR="${ROOT_DIR}/picker_out_internal_dollar"
PROJECT_DIR="${OUTPUT_DIR}/dpi_dollar"
BUILD_LOG="${OUTPUT_DIR}/build.log"

blue "[export-internal-dollar-python] Exporting a DPI signal whose SV name contains '$'"
rm -rf "${OUTPUT_DIR}"
"${PICKER_BIN}" export \
  "${ROOT_DIR}/test/data/internal_dollar.sv" \
  --autobuild false \
  --sdir "${ROOT_DIR}/template" \
  --sname dpi_dollar \
  --tdir "${PROJECT_DIR}" \
  --lang python \
  --sim verilator \
  --internal "${ROOT_DIR}/test/data/internal_dollar.yaml"

DPI_C_SYMBOL="$(python3 - "${PROJECT_DIR}/dpi_dollar_top.sv" <<'PY'
import re
import sys

source = open(sys.argv[1], encoding="utf-8").read()
pattern = (
    r'export "DPI-C" function '
    r'(get_pickerdpix6470695f646f6c6c61722e7369672431xx[A-Za-z0-9_]+);'
)
matches = re.findall(pattern, source)
if len(matches) != 1:
    raise SystemExit(f"expected one collision-safe dollar-sign DPI symbol, found {len(matches)}")
print(matches[0])
PY
)"

cp "${ROOT_DIR}/test/data/internal_dollar.py" "${PROJECT_DIR}/python/example.py"

blue "[export-internal-dollar-python] Building and reading the generated internal signal"
set +e
make -C "${PROJECT_DIR}" EXAMPLE=ON -j"$(nproc)" 2>&1 | tee "${BUILD_LOG}"
make_status=${PIPESTATUS[0]}
set -e

if [[ ${make_status} -ne 0 ]]; then
  red "[export-internal-dollar-python] Generated project build failed"
  exit "${make_status}"
fi
if ! grep -Fq "DPI_INTERNAL_DOLLAR_OK" "${BUILD_LOG}"; then
  red "[export-internal-dollar-python] Generated DUT did not read the dollar-sign signal"
  exit 1
fi

DPI_LIB="$(find "${PROJECT_DIR}" -maxdepth 1 -type f -name 'libUTdpi_dollar.*' -print -quit)"
if [[ -z "${DPI_LIB}" ]]; then
  red "[export-internal-dollar-python] Generated DUT library was not found"
  exit 1
fi

python3 - "${DPI_LIB}" "${DPI_C_SYMBOL}" <<'PY'
import ctypes
import sys

library_path, symbol = sys.argv[1:]
library = ctypes.CDLL(library_path)
try:
    getattr(library, symbol)
except AttributeError as exc:
    raise SystemExit(f"generated DPI-C symbol is missing from final library: {symbol}") from exc
print(f"DPI_INTERNAL_DOLLAR_SYMBOL={symbol}")
PY

green "[export-internal-dollar-python] OK"
