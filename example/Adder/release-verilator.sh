#!/bin/bash


if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

rm -rf mcv_out_adder/
mcv example/Adder/Adder.v -w Adder.fst -S Adder $@ -t mcv_out_adder
cp example/Adder/example.cpp mcv_out_adder/cpp/

cd mcv_out_adder && make
