#!/bin/bash

# This script is used to release the XDummyCache library.

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/CacheSignalCFG"

# run cache codegen
rm -rf "$OUT_DIR"
# Use file list and explicit sname to locate top module in list
./build/bin/picker export --autobuild false --rw mem_direct \
    --sname Cache \
    --tname CacheSignalCFG \
    --fs example/CacheSignalCFG/Cache.txt \
    -w cache.vcd --sim verilator --tdir "${OUT_ROOT}/" "$@"

if [[ $* == *"python"* ]]; then
    cp example/CacheSignalCFG/example.py "$OUT_DIR/python/"
elif [[ $* == *"java"* ]]; then
    cp example/CacheSignalCFG/example.java "$OUT_DIR/java/"
elif [[ $* == *"scala"* ]]; then
    cp example/CacheSignalCFG/example.scala "$OUT_DIR/scala/"
elif [[ $* == *"golang"* ]]; then
    cp example/CacheSignalCFG/example.go "$OUT_DIR/golang/"
elif [[ $* == *"lua"* ]]; then
    cp example/CacheSignalCFG/example.lua "$OUT_DIR/lua/"
else
    cp example/CacheSignalCFG/example.cpp "$OUT_DIR/cpp/"
fi

cd "$OUT_DIR" && make EXAMPLE=ON


# --autobuild true: auto build and test
