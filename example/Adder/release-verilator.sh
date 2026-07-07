#!/bin/bash

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/Adder"

rm -rf "$OUT_DIR"
./build/bin/picker export example/Adder/Adder.v --autobuild false -w Adder.fst --sname Adder --tdir "$OUT_DIR" --sdir template "$@"
# if python in $@, then it will generate python binding
if [[ $* == *"python"* ]]; then
    cp example/Adder/example.py "$OUT_DIR/python/"
elif [[ $* == *"java"* ]]; then
    cp example/Adder/example.java "$OUT_DIR/java/"
elif [[ $* == *"scala"* ]]; then
    cp example/Adder/example.scala "$OUT_DIR/scala/"
elif [[ $* == *"golang"* ]]; then
    cp example/Adder/example.go "$OUT_DIR/golang/"
elif [[ $* == *"lua"* ]]; then
    cp example/Adder/example.lua "$OUT_DIR/lua/"
else
    cp example/Adder/example.cpp "$OUT_DIR/cpp/"
fi

cd "$OUT_DIR" && make EXAMPLE=ON
