#!/bin/bash

rm -rf ./picker_out/XDummyCache
./build/bin/picker export example/XDummyCache/Cache.sv --autobuild true --sim vcs -w XDumyCache.fsdb --sname XDumyCache --tdir ./picker_out/XDummyCache --sdir template $@

if [[ $@ == *"python"* ]]; then
    cd ./picker_out/XDummyCache
    echo "'cannot allocate memory in static TLS block'error for VCS is expected, please ignore it"
    LD_PRELOAD=./libUTXDumyCache.so python3 example.py
fi
