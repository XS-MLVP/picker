#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/common.sh"

PICKER_BIN="$(resolve_picker)"
ROOT_DIR="${ROOT_DIR:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/picker-export-filelist.XXXXXX")"
trap 'rm -rf "${TMP_DIR}"' EXIT

mkdir -p "${TMP_DIR}/list" "${TMP_DIR}/rtl/inc"

cat > "${TMP_DIR}/rtl/inc/inner.svh" <<'EOF'
`define WIDTH_BASE 4
EOF

cat > "${TMP_DIR}/rtl/inc/outer.svh" <<'EOF'
`include "inner.svh"
`define WIDTH (`WIDTH_BASE + 4)
EOF

cat > "${TMP_DIR}/rtl/top.sv" <<'EOF'
`include "inc/outer.svh"
module FileListTop(
    input logic [`WIDTH-1:0] a,
    output logic [$clog2(16)-1:0] b
);
endmodule
EOF

cat > "${TMP_DIR}/list/design.f" <<'EOF'
../rtl/top.sv
EOF

OUT_DIR="${TMP_DIR}/out/FileListTop"

blue "[export-filelist] Exporting from a filelist with a minimal tool PATH"
env PATH="/usr/bin:/bin" "${PICKER_BIN}" export \
  --autobuild false \
  --sdir "${ROOT_DIR}/template" \
  --sname FileListTop \
  --fs "${TMP_DIR}/list/design.f" \
  --tdir "${OUT_DIR}" \
  --lang python \
  --sim verilator

test -d "${OUT_DIR}"
test -f "${OUT_DIR}/CMakeLists.txt"
test -d "${OUT_DIR}/python"

green "[export-filelist] OK"
