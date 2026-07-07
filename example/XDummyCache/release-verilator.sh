#!/bin/bash

# This script is used to release the XDummyCache library.

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/XDummyCache"

# rebuild picker if with parameter --rebuild
if [ "$1" == "--rebuild" ]; then
    rm -rf build
    cmake . -Bbuild
    cd build && make -j`nproc` && cd ../
fi


# run cache codegen
rm -rf temp 
rm -rf "$OUT_DIR"
./build/bin/picker export example/XDummyCache/Cache.sv --autobuild true --sdir ./template --tdir "$OUT_DIR" --sname XDumyCache --tname Cache -w Cache.fst --sim verilator "$@"
