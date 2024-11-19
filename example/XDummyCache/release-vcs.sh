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
./build/bin/picker export example/XDummyCache/XDummyCache.v --autobuild true --sim vcs -w XDummyCache.fsdb --sname XDummyCache --tdir ./picker_out/temp --sdir template $@

if [[ $@ == *"python"* ]]; then
    cd ./picker_out/temp
    echo "'cannot allocate memory in static TLS block'error for VCS is expected, please ignore it"
    LD_PRELOAD=./libUTXDummyCache.so python3 example.py
fi