#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"

blue "[pack-python] Generating UVM pack example for Python"
WORKDIR="${ROOT_DIR}/pack_example"
rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cp "${ROOT_DIR}/example/Pack/adder_trans.sv" "${WORKDIR}/"

pushd "${WORKDIR}" >/dev/null
"${PICKER_BIN}" pack adder_trans -e

blue "[pack-python] Building Python2UVM and UVM2Python examples"
cp -r "${ROOT_DIR}/example/Pack/UVM2Python" .
cp -r "${ROOT_DIR}/example/Pack/Python2UVM" .
cp adder_trans/example/Makefile UVM2Python/
cp adder_trans/example/Makefile Python2UVM/

make -C UVM2Python -j"$(nproc)"
make -C Python2UVM -j"$(nproc)"
popd >/dev/null

green "[pack-python] OK"
