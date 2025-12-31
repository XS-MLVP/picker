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

./build/bin/picker pack --from-rtl example/Adder/Adder.v -n Adder -d -e

rm -rf uvmpy/example.sv
rm -rf uvmpy/example.py

cp example/Adder/example.py uvmpy/
cp example/Adder/example-uvm.sv uvmpy/

mv uvmpy picker_out_adder

cd picker_out_adder && make
