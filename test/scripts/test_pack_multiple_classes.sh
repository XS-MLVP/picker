#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

PICKER_BIN="$(resolve_picker)"
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/picker-pack-multi-class.XXXXXX")"
trap 'rm -rf "${TMP_DIR}"' EXIT

cat > "${TMP_DIR}/dual_trans.sv" <<'EOF'
class first_trans extends uvm_sequence_item;
    bit a;
    function new(string name = "first_trans");
        super.new(name);
    endfunction
endclass

class second_trans extends uvm_sequence_item;
    bit b;
    function new(string name = "second_trans");
        super.new(name);
    endfunction
endclass
EOF

set +e
OUTPUT=$(env PATH="/usr/bin:/bin" "${PICKER_BIN}" pack "${TMP_DIR}/dual_trans.sv" 2>&1)
STATUS=$?
set -e

if [[ ${STATUS} -eq 0 ]]; then
  red "[pack-multi-class] expected picker pack to fail when a file defines multiple transaction classes"
  exit 1
fi

grep -qi "Multiple transaction classes" <<<"${OUTPUT}"
green "[pack-multi-class] OK"
