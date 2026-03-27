#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

PICKER_BIN="$(resolve_picker)"
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/picker-pack-transaction.XXXXXX")"
trap 'rm -rf "${TMP_DIR}"' EXIT

blue "[pack-transaction] Verifying transaction pack flow with a minimal tool PATH"

cp "${ROOT_DIR}/example/Pack/adder_trans.sv" "${TMP_DIR}/"
pushd "${TMP_DIR}" >/dev/null
env PATH="/usr/bin:/bin" "${PICKER_BIN}" pack adder_trans -e
test -f "${TMP_DIR}/uvmpy/adder_trans/xagent.sv"
test -f "${TMP_DIR}/uvmpy/example.sv"
popd >/dev/null

blue "[pack-transaction] Verifying multi-transaction filelist flow"

mkdir -p "${TMP_DIR}/multi"
cp "${ROOT_DIR}/example/Pack/03_MultiTrans/alu_op.sv" "${TMP_DIR}/multi/"
cp "${ROOT_DIR}/example/Pack/03_MultiTrans/alu_result.sv" "${TMP_DIR}/multi/"
cp "${ROOT_DIR}/example/Pack/03_MultiTrans/filelist.txt" "${TMP_DIR}/multi/"
pushd "${TMP_DIR}/multi" >/dev/null
env PATH="/usr/bin:/bin" "${PICKER_BIN}" pack -f filelist.txt -n ALU -d
test -f "${TMP_DIR}/multi/ALU/xagent.sv"
test -f "${TMP_DIR}/multi/ALU/utils_pkg.sv"
popd >/dev/null

green "[pack-transaction] OK"
