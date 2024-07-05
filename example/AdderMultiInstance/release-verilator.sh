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
./build/bin/picker export example/AdderMultiInstance/Adder.v --autobuild=true -w Adder.fst --sname Adder --tdir picker_out_adder --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/AdderMultiInstance/example.py picker_out_adder/python/
elif [[ $@ == *"java"* ]]; then
    cp example/AdderMultiInstance/example.java picker_out_adder/java/
else
    cp example/AdderMultiInstance/example.cpp picker_out_adder/cpp/
fi

cd picker_out_adder && make EXAMPLE=ON
