#!/bin/bash
set -e

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="$(realpath -m "${OUT_ROOT}/XDummyCache")"

rm -rf "$OUT_DIR"
./build/bin/picker export example/XDummyCache/Cache.sv --autobuild true --sim vcs -w XDumyCache.fsdb --sname XDumyCache --tdir "$OUT_DIR" --sdir template "$@"
