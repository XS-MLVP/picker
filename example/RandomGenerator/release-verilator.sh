#!/bin/bash

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/RandomGenerator"

rm -rf "$OUT_DIR"
./build/bin/picker export example/RandomGenerator/RandomGenerator.v --autobuild false -w RandomGenerator.fst "$@" --tdir "$OUT_DIR"
# if python in $@, then it will generate python binding
if [[ $* == *"python"* ]]; then
    cp example/RandomGenerator/example.py "$OUT_DIR/python/"
elif [[ $* == *"java"* ]]; then
    cp example/RandomGenerator/example.java "$OUT_DIR/java/"
elif [[ $* == *"scala"* ]]; then
    cp example/RandomGenerator/example.scala "$OUT_DIR/scala/"
elif [[ $* == *"golang"* ]]; then
    cp example/RandomGenerator/example.go "$OUT_DIR/golang/"
else
    cp example/RandomGenerator/example.cpp "$OUT_DIR/cpp/"
fi

cd "$OUT_DIR" && make EXAMPLE=ON
