#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

require_cmd cmake

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

OUTDIR="${ROOT_DIR}/picker_out_GSIM/NutShell"
FIR="${ROOT_DIR}/example/GSim/NutShell/SimTop-nutshell.fir"
ARCHIVE="${ROOT_DIR}/example/GSim/NutShell/SimTop-nutshell.tar.bz2"

blue "[export-gsim-smoke] Preparing NutShell FIR"
if [[ ! -f "${FIR}" ]]; then
  tar -xf "${ARCHIVE}" -C "${ROOT_DIR}/example/GSim/NutShell/"
fi

blue "[export-gsim-smoke] Exporting with --sim gsim (no build)"
rm -rf "${OUTDIR}"
"${PICKER_BIN}" export \
  "${FIR}" \
  --rw mem_direct \
  --autobuild false \
  --sim gsim \
  --tdir "${OUTDIR}"

test -d "${OUTDIR}"
test -f "${OUTDIR}/Makefile"

green "[export-gsim-smoke] OK"

