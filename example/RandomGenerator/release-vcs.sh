#!/bin/bash


if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

rm -rf picker_out_RandomGenerator/
./build/bin/picker export example/RandomGenerator/RandomGenerator.v --autobuild false --sim vcs -w RandomGenerator.fsdb --sname RandomGenerator --tdir picker_out_RandomGenerator --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/RandomGenerator/example.py picker_out_RandomGenerator/python/
elif [[ $@ == *"java"* ]]; then
    cp example/RandomGenerator/example.java picker_out_RandomGenerator/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/RandomGenerator/example.scala picker_out_RandomGenerator/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/RandomGenerator/example.go picker_out_RandomGenerator/golang/
else
    cp example/RandomGenerator/example.cpp picker_out_RandomGenerator/cpp/
fi

cd picker_out_RandomGenerator && make EXAMPLE=ON

if [[ $@ == *"python"* ]]; then
    echo "'cannot allocate memory in static TLS block'error for VCS is expected, please ignore it"
    LD_PRELOAD=./libUTRandomGenerator.so python3 example.py
fi