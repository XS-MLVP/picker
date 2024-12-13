#!/bin/bash


if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

rm -rf picker_out/
./build/bin/picker export example/MultiClock/multi_clock.v --autobuild false -w mc.fst --tdir picker_out/MultiClock --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/MultiClock/example.py picker_out/MultiClock/python/
else
    echo "This example only supports python now, please use --lang python"
    exit
fi

cd picker_out/MultiClock && make EXAMPLE=ON
