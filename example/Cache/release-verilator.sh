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
rm -rf picker_out
./build/bin/picker export --autobuild false example/Cache/Cache.v --fs example/Cache/Test.v -w cache.vcd --sim verilator --tdir ./picker_out/ $@

# build cache
cd picker_out/Cache && make
# echo "build cache done, all you need has been generated in temp/release"
