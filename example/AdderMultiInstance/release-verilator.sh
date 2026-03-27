#!/bin/bash

rm -rf picker_out_adder/
./build/bin/picker export example/AdderMultiInstance/Adder.v --autobuild false -w Adder.fst --sname Adder --tdir picker_out_adder --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/AdderMultiInstance/example.py picker_out_adder/python/
else
    echo "Only example.py find! Use --lang python"
    exit -1
fi

cd picker_out_adder && make EXAMPLE=ON
