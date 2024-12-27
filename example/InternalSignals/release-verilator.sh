#!/bin/bash

# This script is used to release the XDummyCache library.

# has verible?
if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

# run cache codegen
rm -rf ./picker_out 
./build/bin/picker export example/InternalSignals/vpi.v --autobuild false --sdir ./template --vpi --sname vpi --tdir ./picker_out/InternalSignals --sim verilator $@

if [ $? -ne 0 ]; then
    echo "codegen failed"
    exit
fi

# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/InternalSignals/example.py picker_out/InternalSignals/python/
elif [[ $@ == *"java"* ]]; then
    cp example/InternalSignals/example.java picker_out/InternalSignals/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/InternalSignals/example.scala picker_out/InternalSignals/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/InternalSignals/example.go picker_out/InternalSignals/golang/
elif [[ $@ == *"lua"* ]]; then
    cp example/InternalSignals/example.lua picker_out/InternalSignals/lua/
else
    cp example/InternalSignals/example.cpp picker_out/InternalSignals/cpp/
fi

cd picker_out/InternalSignals && make EXAMPLE=ON


