#!/bin/bash

# This script is used to release the XDummyCache library.

# rebuild picker if with parameter --rebuild
if [ "$1" == "--rebuild" ]; then
    rm -rf build
    cmake . -Bbuild
    cd build && make -j`nproc` && cd ../
fi


# run cache codegen
rm -rf ./picker_out/XDummyCache
./build/bin/picker export example/XDummyCache/Cache.sv --autobuild true --sdir ./template --tdir ./picker_out/XDummyCache --sname XDumyCache --tname Cache -w Cache.fst --sim verilator $@
