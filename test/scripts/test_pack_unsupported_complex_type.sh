#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

PICKER_BIN="$(resolve_picker)"
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/picker-pack-unsupported.XXXXXX")"
trap 'rm -rf "${TMP_DIR}"' EXIT

cat > "${TMP_DIR}/bad_trans.sv" <<'EOF'
class bad_trans extends uvm_sequence_item;
    string name;
    function new(string name = "bad_trans");
        super.new(name);
    endfunction
endclass
EOF

set +e
OUTPUT=$(env PATH="/usr/bin:/bin" "${PICKER_BIN}" pack "${TMP_DIR}/bad_trans.sv" 2>&1)
STATUS=$?
set -e

if [[ ${STATUS} -eq 0 ]]; then
  red "[pack-unsupported] expected picker pack to fail for unsupported member type"
  exit 1
fi

grep -qi "unsupported type" <<<"${OUTPUT}"
green "[pack-unsupported] OK"
