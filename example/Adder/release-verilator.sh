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
./build/bin/picker export example/Adder/Adder.v --autobuild false -w Adder.fst --sname Adder --tdir picker_out_adder/Adder --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/Adder/example.py picker_out_adder/Adder/python/
elif [[ $@ == *"java"* ]]; then
    cp example/Adder/example.java picker_out_adder/Adder/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/Adder/example.scala picker_out_adder/Adder/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/Adder/example.go picker_out_adder/Adder/golang/
else
    cp example/Adder/example.cpp picker_out_adder/Adder/cpp/
fi

cd picker_out_adder/Adder && make EXAMPLE=ON
