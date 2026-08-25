#!/bin/bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
OUT_ROOT="${OUT_ROOT:-$ROOT_DIR/output}"
OUT_DIR="$(realpath -m "$OUT_ROOT/Coverage")"

rm -rf "$OUT_DIR"
"$ROOT_DIR/build/bin/picker" export \
    "$ROOT_DIR/example/DualPortStackCb/dual_port_stack.v" \
    --autobuild true \
    --sim verilator \
    --sname dual_port_stack \
    --tdir "$OUT_DIR" \
    --sdir "$ROOT_DIR/template" \
    --coverage \
    "$@"

cp "$ROOT_DIR/example/Coverage/example.py" "$OUT_DIR/example.py"

cd "$OUT_DIR"
python3 example.py
