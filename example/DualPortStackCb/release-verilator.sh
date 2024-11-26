#!/bin/bash


if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

rm -rf picker_out_dps/

./build/bin/picker export example/DualPortStackCb/dual_port_stack.v --autobuild false -w DualPortStackCb.fst $@ --tdir picker_out_dps
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/DualPortStackCb/example.py picker_out_dps/python/
elif [[ $@ == *"java"* ]]; then
    cp example/DualPortStackCb/example.java picker_out_dps/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/DualPortStackCb/example.scala picker_out_dps/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/DualPortStackCb/example.go picker_out_dps/golang/
else
    cp example/DualPortStackCb/example.cpp picker_out_dps/cpp/
fi

cd picker_out_dps && make EXAMPLE=ON
