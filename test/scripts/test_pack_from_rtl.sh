#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

PICKER_BIN="$(resolve_picker)"
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/picker-pack-from-rtl.XXXXXX")"
trap 'rm -rf "${TMP_DIR}"' EXIT

blue "[pack-from-rtl] Verifying --from-rtl flow with a minimal tool PATH"

pushd "${TMP_DIR}" >/dev/null
env PATH="/usr/bin:/bin" "${PICKER_BIN}" pack --from-rtl "${ROOT_DIR}/example/Adder/Adder.v" -n Adder -d -e
test -f "${TMP_DIR}/uvmpy/Adder/Adder_trans.sv"
test -f "${TMP_DIR}/uvmpy/Adder/xagent.sv"
popd >/dev/null

green "[pack-from-rtl] OK"
