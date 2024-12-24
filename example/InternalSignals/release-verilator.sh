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
./build/bin/picker export example/InternalSignals/vpi.v --autobuild false --sdir ./template --vpi --sname vpi --tdir ./picker_out/InterlSinals --sim verilator --lang python
cp example/InternalSignals/example.py picker_out/InterlSinals/python/
cd picker_out/InterlSinals && make EXAMPLE=ON
