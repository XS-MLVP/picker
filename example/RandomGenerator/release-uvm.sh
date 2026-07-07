#!/bin/bash

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/RandomGenerator"

rm -rf "$OUT_DIR"

./build/bin/picker pack --from-rtl example/RandomGenerator/RandomGenerator.v -d -e

rm -rf uvmpy/example.sv
rm -rf uvmpy/example.py

cp example/RandomGenerator/example.py uvmpy/
cp example/RandomGenerator/example-uvm.sv uvmpy/
cp example/RandomGenerator/RandomGenerator.v uvmpy/

mv uvmpy "$OUT_DIR"

cd "$OUT_DIR" && make
