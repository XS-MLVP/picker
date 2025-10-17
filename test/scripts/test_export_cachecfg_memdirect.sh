#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd verible-verilog-syntax
require_cmd cmake
require_cmd python3

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

OUTDIR="${ROOT_DIR}/picker_out/CacheCFG"

blue "[export-cachecfg] Exporting Cache with --fs Cache.txt and --sname Cache"
rm -rf "${OUTDIR}"
"${PICKER_BIN}" export \
  --autobuild false \
  --rw mem_direct \
  --sname Cache \
  --tname CacheCFG \
  --fs "${ROOT_DIR}/example/CacheSignalCFG/Cache.txt" \
  --tdir  "${ROOT_DIR}/picker_out/" \
  --lang python \
  --sim verilator

cp "${ROOT_DIR}/example/CacheSignalCFG/example.py" "${OUTDIR}/python/"

blue "[export-cachecfg] Building generated project"
make -C "${OUTDIR}" EXAMPLE=ON -j"$(nproc 2>/dev/null || echo 2)"

green "[export-cachecfg] OK"

