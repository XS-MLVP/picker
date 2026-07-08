#!/bin/bash
set -e

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="$(realpath -m "${OUT_ROOT}/MultiFileRTL")"
LANGUAGE_ARGS=()

rm -rf "$OUT_DIR"
if [[ $* == *"python"* ]]; then
    LANGUAGE_ARGS+=(--lang python)
fi

./build/bin/picker export \
  --fs example/MultiFileRTL/design.f \
  --autobuild true \
  --sim vcs \
  --verdi-mode modern \
  -w MultiFileRTLTop.fsdb \
  --sname MultiFileRTLTop \
  --tdir "$OUT_DIR" \
  --sdir template \
  "${LANGUAGE_ARGS[@]}" \
  "$@"

if [[ $* == *"python"* ]]; then
    cp example/MultiFileRTL/example.py "$OUT_DIR"
    cd "$OUT_DIR"
    python3 example.py
fi
