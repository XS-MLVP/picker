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

# rebuild picker if with parameter --rebuild
if [ "$1" == "--rebuild" ]; then
    rm -rf build
    cmake . -Bbuild
    cd build && make -j`nproc` && cd ../
fi


# run cache codegen
rm -rf temp 
./build/bin/picker export example/XDummyCache/Cache.sv --autobuild true --sdir ./template --tdir ./picker_out/temp  --sname XDumyCache --tname Cache -w Cache.fst --sim verilator $@
