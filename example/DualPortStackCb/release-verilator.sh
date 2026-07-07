#!/bin/bash

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/DualPortStackCb"

rm -rf "$OUT_DIR"
./build/bin/picker export example/DualPortStackCb/dual_port_stack.v --autobuild false -w DualPortStackCb.fst "$@" --tdir "$OUT_DIR"
# if python in $@, then it will generate python binding
if [[ $* == *"python"* ]]; then
    if [[ "$async" != "" ]]; then
        cp example/DualPortStackCb/example_async.py "$OUT_DIR/python/example.py"
    else
        cp example/DualPortStackCb/example.py "$OUT_DIR/python/"
    fi
elif [[ $* == *"java"* ]]; then
    cp example/DualPortStackCb/example.java "$OUT_DIR/java/"
elif [[ $* == *"scala"* ]]; then
    cp example/DualPortStackCb/example.scala "$OUT_DIR/scala/"
elif [[ $* == *"golang"* ]]; then
    cp example/DualPortStackCb/example.go "$OUT_DIR/golang/"
else
    cp example/DualPortStackCb/example.cpp "$OUT_DIR/cpp/"
fi

cd "$OUT_DIR" && make EXAMPLE=ON
