#!/bin/bash


if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

rm -rf picker_out_XDummyCache/
./build/bin/picker export example/XDummyCache/XDummyCache.v --autobuild false --sim vcs -w XDummyCache.fsdb --sname XDummyCache --tdir picker_out_XDummyCache --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/XDummyCache/example.py picker_out_XDummyCache/python/
elif [[ $@ == *"java"* ]]; then
    cp example/XDummyCache/example.java picker_out_XDummyCache/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/XDummyCache/example.scala picker_out_XDummyCache/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/XDummyCache/example.go picker_out_XDummyCache/golang/
else
    cp example/XDummyCache/example.cpp picker_out_XDummyCache/cpp/
fi

cd picker_out_XDummyCache && make EXAMPLE=ON

if [[ $@ == *"python"* ]]; then
    echo "'cannot allocate memory in static TLS block'error for VCS is expected, please ignore it"
    LD_PRELOAD=./UT_XDummyCache/libUTXDummyCache.so python3 example.py
fi