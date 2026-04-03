#!/bin/bash

rm -rf picker_out/AdderMultiInstance
./build/bin/picker export example/AdderMultiInstance/Adder.v --autobuild false -w Adder.fst --sname Adder --tdir picker_out/AdderMultiInstance --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/AdderMultiInstance/example.py picker_out/AdderMultiInstance/python/
else
    echo "Only example.py find! Use --lang python"
    exit -1
fi

cd picker_out/AdderMultiInstance && make EXAMPLE=ON
