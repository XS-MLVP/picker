#!/bin/bash


if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

rm -rf picker_out_Cache/
./build/bin/picker export example/Cache/Cache.v --autobuild false --sim vcs -w Cache.fsdb --sname Cache --tdir picker_out_Cache --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/Cache/example.py picker_out_Cache/python/
elif [[ $@ == *"java"* ]]; then
    cp example/Cache/example.java picker_out_Cache/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/Cache/example.scala picker_out_Cache/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/Cache/example.go picker_out_Cache/golang/
else
    cp example/Cache/example.cpp picker_out_Cache/cpp/
fi

cd picker_out_Cache && make EXAMPLE=ON

if [[ $@ == *"python"* ]]; then
    echo "'cannot allocate memory in static TLS block'error for VCS is expected, please ignore it"
    LD_PRELOAD=./UT_Cache/libUTCache.so python3 example.py
fi