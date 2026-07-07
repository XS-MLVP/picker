#!/bin/bash

# This script is used to release the XDummyCache library.

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/Cache"

# run cache codegen
rm -rf "$OUT_DIR"
./build/bin/picker export --autobuild true example/Cache/Cache.v --fs example/Cache/Test.v -w cache.vcd --sim verilator --tdir "${OUT_ROOT}/" "$@"

# --autobuild true: auto build and test
