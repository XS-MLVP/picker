#!/bin/bash

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/Adder"

rm -rf "$OUT_DIR"

./build/bin/picker pack --from-rtl example/Adder/Adder.v -n Adder -d -e

rm -rf uvmpy/example.sv
rm -rf uvmpy/example.py

cp example/Adder/example.py uvmpy/
cp example/Adder/example-uvm.sv uvmpy/

mv uvmpy "$OUT_DIR"

cd "$OUT_DIR" && make
