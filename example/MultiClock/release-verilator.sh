#!/bin/bash

rm -rf picker_out/MultiClock
./build/bin/picker export example/MultiClock/multi_clock.v --autobuild false -w mc.fst --tdir picker_out/MultiClock --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/MultiClock/example.py picker_out/MultiClock/python/
else
    echo "This example only supports python now, please use --lang python"
    exit
fi

cd picker_out/MultiClock && make EXAMPLE=ON
