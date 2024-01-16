#!/bin/bash


if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

rm -rf picker_out_rmg/
picker example/RandomGenerator/RandomGenerator.v -w RandomGenerator.fst -S RandomGenerator $@ -t picker_out_rmg
cp example/RandomGenerator/example.cpp picker_out_rmg/cpp/

cd picker_out_rmg && make
