#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/picker-export-complex-port.XXXXXX")"
trap 'rm -rf "${TMP_DIR}"' EXIT

cat > "${TMP_DIR}/complex_port.sv" <<'EOF'
module ComplexPort(
    input logic [3:0] data [1:0]
);
endmodule
EOF

STDOUT_LOG="${TMP_DIR}/stdout.log"
STDERR_LOG="${TMP_DIR}/stderr.log"

blue "[export-complex-port] Verifying unsupported complex ports fail fast"
if env PATH="/usr/bin:/bin" "${PICKER_BIN}" export \
  "${TMP_DIR}/complex_port.sv" \
  --autobuild false \
  --sdir "${ROOT_DIR}/template" \
  --sname ComplexPort \
  --tdir "${TMP_DIR}/out/ComplexPort" \
  --lang python \
  --sim verilator >"${STDOUT_LOG}" 2>"${STDERR_LOG}"; then
  red "[export-complex-port] picker export unexpectedly succeeded"
  exit 1
fi

grep -qi "unsupported type" "${STDERR_LOG}"
grep -qi "scalar and single-dimension packed" "${STDERR_LOG}"

green "[export-complex-port] OK"
