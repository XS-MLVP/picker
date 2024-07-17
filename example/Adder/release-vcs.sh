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
./build/bin/picker export example/Adder/Adder.v --autobuild false --sim vcs -w Adder.fsdb --sname Adder --tdir picker_out_adder --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/Adder/example.py picker_out_adder/python/
elif [[ $@ == *"java"* ]]; then
    cp example/Adder/example.java picker_out_adder/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/Adder/example.scala picker_out_adder/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/Adder/example.go picker_out_adder/golang/
else
    cp example/Adder/example.cpp picker_out_adder/cpp/
fi

cd picker_out_adder && make EXAMPLE=ON

if [[ $@ == *"python"* ]]; then
    echo "'cannot allocate memory in static TLS block'error for VCS is expected, please ignore it"
    LD_PRELOAD=./UT_Adder/libUTAdder.so python3 example.py
fi