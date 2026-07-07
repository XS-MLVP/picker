#!/bin/bash

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/MultiClock"

rm -rf "$OUT_DIR"
./build/bin/picker export example/MultiClock/multi_clock.v --autobuild false -w mc.fst --tdir "$OUT_DIR" --sdir template "$@"
# if python in $@, then it will generate python binding
if [[ $* == *"python"* ]]; then
    cp example/MultiClock/example.py "$OUT_DIR/python/"
else
    echo "This example only supports python now, please use --lang python"
    exit
fi

cd "$OUT_DIR" && make EXAMPLE=ON
