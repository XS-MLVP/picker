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

./build/bin/picker pack --from-rtl example/RandomGenerator/RandomGenerator.v -d -e

rm -rf uvmpy/example.sv
rm -rf uvmpy/example.py

cp example/RandomGenerator/example.py uvmpy/
cp example/RandomGenerator/example-uvm.sv uvmpy/
cp example/RandomGenerator/RandomGenerator.v uvmpy/

mv uvmpy picker_out_rmg

cd picker_out_rmg && make
