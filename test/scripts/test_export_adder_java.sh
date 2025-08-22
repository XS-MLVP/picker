#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd verible-verilog-syntax
require_cmd cmake
require_cmd java
require_cmd javac

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

OUTROOT="${ROOT_DIR}/picker_out_adder_java"
OUTDIR="${OUTROOT}/Adder"

blue "[export-java] Exporting Adder.v to Java project"
rm -rf "${OUTROOT}"
"${PICKER_BIN}" export \
  "${ROOT_DIR}/example/Adder/Adder.v" \
  --autobuild false \
  --sdir "${ROOT_DIR}/template" \
  --sname Adder \
  --tdir  "${OUTDIR}" \
  --lang java \
  --sim verilator

cp "${ROOT_DIR}/example/Adder/example.java" "${OUTDIR}/java/"

blue "[export-java] Building generated Java project"
make -C "${OUTDIR}" EXAMPLE=ON -j"$(nproc)"

green "[export-java] OK"

