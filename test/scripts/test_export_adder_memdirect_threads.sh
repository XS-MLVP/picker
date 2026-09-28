#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd cmake
require_cmd ctest
require_cmd grep
require_cmd make
require_cmd verilator

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

blue "[export-adder-memdirect-threads] Running parser regression"
cmake --build "${ROOT_DIR}/build" --target test_parser_verilator_root_unit --parallel "$(nproc 2>/dev/null || echo 2)"
ctest --test-dir "${ROOT_DIR}/build" --output-on-failure -R '^test_parser_verilator_root_unit$'

VERILATOR_VERSION_OUTPUT="$(verilator --version)"
read -r _ VERILATOR_VERSION _ <<<"${VERILATOR_VERSION_OUTPUT}"
VERILATOR_MAJOR="${VERILATOR_VERSION%%.*}"
VERILATOR_MINOR="${VERILATOR_VERSION#*.}"

if ((VERILATOR_MAJOR < 5 || (VERILATOR_MAJOR == 5 && 10#${VERILATOR_MINOR} < 50))); then
  green "[export-adder-memdirect-threads] SKIP: requires Verilator 5.050+"
  exit 0
fi

TEST_TMP_ROOT="$(mktemp -d)"
OUTDIR="${TEST_TMP_ROOT}/export"
trap 'rm -rf "${TEST_TMP_ROOT}"' EXIT

blue "[export-adder-memdirect-threads] Using ${VERILATOR_VERSION_OUTPUT}"
"${PICKER_BIN}" export \
  "${ROOT_DIR}/example/Adder/Adder.v" \
  --autobuild false \
  --sname Adder \
  --tname Adder \
  --rw mem_direct \
  --lang cpp \
  --tdir "${OUTDIR}" \
  -w "${OUTDIR}/trace.fst" \
  -V "--threads;2"

blue "[export-adder-memdirect-threads] Building generated project"
make -C "${OUTDIR}" compile NPROC="$(nproc 2>/dev/null || echo 2)"

ROOT_HEADER="${OUTDIR}/build/DPIAdder/VAdder___024root.h"
OFFSET_YAML="${OUTDIR}/mem_direct/Adder_offset.yaml"

test -f "${ROOT_HEADER}"
test -f "${OFFSET_YAML}"
if grep -Eq 'alignas\(VL_CACHE_LINE_BYTES\).*Adder_top__DOT__cin' "${ROOT_HEADER}"; then
  blue "[export-adder-memdirect-threads] Verifying generated alignas declarations"
else
  blue "[export-adder-memdirect-threads] Verilator did not emit an aligned cin declaration"
fi
grep -Fq 'Adder_top__DOT__cin' "${ROOT_HEADER}"
grep -Fq 'source: "Adder_top.cin"' "${OFFSET_YAML}"

green "[export-adder-memdirect-threads] OK"
