#!/bin/bash

rm -rf picker_out_Cache/
./build/bin/picker export example/Cache/Cache.v --autobuild true --sim vcs -w Cache.fsdb --sname Cache --tdir picker_out_Cache --sdir template $@

cd picker_out_Cache && make EXAMPLE=ON

if [[ $@ == *"python"* ]]; then
    echo "'cannot allocate memory in static TLS block'error for VCS is expected, please ignore it"
    LD_PRELOAD=./libUTCache.so python3 example.py
fi
