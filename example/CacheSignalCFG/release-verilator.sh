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
rm -rf picker_out/CacheCFG
./build/bin/picker export  example/CacheSignalCFG/Cache.v --autobuild false  --rw mem_direct \
    --tname CacheCFG --fs example/CacheSignalCFG/Test.v \
    -w cache.vcd --sim verilator --tdir ./picker_out/ $@

if [[ $@ == *"python"* ]]; then
    cp example/CacheSignalCFG/example.py picker_out/CacheCFG/python/
elif [[ $@ == *"java"* ]]; then
    cp example/CacheSignalCFG/example.java picker_out/CacheCFG/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/CacheSignalCFG/example.scala picker_out/CacheCFG/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/CacheSignalCFG/example.go picker_out/CacheCFG/golang/
elif [[ $@ == *"lua"* ]]; then
    cp example/CacheSignalCFG/example.lua picker_out/CacheCFG/lua/
else
    cp example/CacheSignalCFG/example.cpp picker_out/CacheCFG/cpp/
fi

cd picker_out/CacheCFG && make EXAMPLE=ON


# --autobuild true: auto build and test
