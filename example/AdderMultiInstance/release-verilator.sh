#!/bin/bash

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/AdderMultiInstance"

rm -rf "$OUT_DIR"
./build/bin/picker export example/AdderMultiInstance/Adder.v --autobuild false -w Adder.fst --sname Adder --tdir "$OUT_DIR" --sdir template "$@"
# if python in $@, then it will generate python binding
if [[ $* == *"python"* ]]; then
    cp example/AdderMultiInstance/example.py "$OUT_DIR/python/"
else
    echo "Only example.py find! Use --lang python"
    exit -1
fi

cd "$OUT_DIR" && make EXAMPLE=ON
