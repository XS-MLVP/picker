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

# rebuild mcv if with parameter --rebuild
if [ "$1" == "--rebuild" ]; then
    rm -rf build
    cmake . -Bbuild
    cd build && make -j`nproc` && cd ../
fi


# run cache codegen
rm -rf mcv_out
# mcv -f example/Cache/Cache.v -s ./template -t ./temp -S Cache -T Cache -w cache.fst --sim verilator
mcv -f example/Cache/Cache.v -w cache.fst 

# build cache
# cd temp && make
# echo "build cache done, all you need has been generated in temp/release"
