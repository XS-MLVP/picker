#!/bin/bash


if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

rm -rf picker_out_adder/
./build/bin/picker export example/AdderMultiInstance/Adder.v --autobuild false -w Adder.fst --sname Adder --tdir picker_out_adder --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/AdderMultiInstance/example.py picker_out_adder/python/
else
    echo "Only example.py find! Use --lang python"
    exit -1
fi

cd picker_out_adder && make EXAMPLE=ON
