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

./build/bin/picker export example/RandomGenerator/RandomGenerator.v --autobuild false -w RandomGenerator.fst $@ --tdir picker_out_rmg
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/RandomGenerator/example.py picker_out_rmg/python/
elif [[ $@ == *"java"* ]]; then
    cp example/RandomGenerator/example.java picker_out_rmg/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/RandomGenerator/example.scala picker_out_rmg/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/RandomGenerator/example.go picker_out_rmg/golang/
else
    cp example/RandomGenerator/example.cpp picker_out_rmg/cpp/
fi

cd picker_out_rmg && make EXAMPLE=ON
