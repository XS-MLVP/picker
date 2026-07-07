#!/bin/bash

# This script is used to release the XDummyCache library.

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/InternalSignals"

# run cache codegen
rm -rf "$OUT_DIR"
./build/bin/picker export example/InternalSignals/vpi.v --autobuild false --sdir ./template --vpi --sname vpi --tdir "$OUT_DIR" --sim verilator "$@"

if [ $? -ne 0 ]; then
    echo "codegen failed"
    exit
fi

# if python in $@, then it will generate python binding
if [[ $* == *"python"* ]]; then
    cp example/InternalSignals/example.py "$OUT_DIR/python/"
elif [[ $* == *"java"* ]]; then
    cp example/InternalSignals/example.java "$OUT_DIR/java/"
elif [[ $* == *"scala"* ]]; then
    cp example/InternalSignals/example.scala "$OUT_DIR/scala/"
elif [[ $* == *"golang"* ]]; then
    cp example/InternalSignals/example.go "$OUT_DIR/golang/"
elif [[ $* == *"lua"* ]]; then
    cp example/InternalSignals/example.lua "$OUT_DIR/lua/"
else
    cp example/InternalSignals/example.cpp "$OUT_DIR/cpp/"
fi

cd "$OUT_DIR" && make EXAMPLE=ON
