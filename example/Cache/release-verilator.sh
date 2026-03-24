#!/bin/bash

# This script is used to release the XDummyCache library.

# run cache codegen
rm -rf picker_out/Cache
./build/bin/picker export --autobuild true example/Cache/Cache.v --fs example/Cache/Test.v -w cache.vcd --sim verilator --tdir ./picker_out/ $@

# --autobuild true: auto build and test
