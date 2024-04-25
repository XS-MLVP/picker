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
picker example/Adder/Adder.v --autobuild=false -w Adder.fst -S Adder $@ -t picker_out_adder
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/Adder/example.py picker_out_adder/python/
else
    cp example/Adder/example.cpp picker_out_adder/cpp/
fi

cd picker_out_adder && make
