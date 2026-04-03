#!/bin/bash

rm -rf picker_out/Adder
./build/bin/picker export example/Adder/Adder.v --autobuild false -w Adder.fst --sname Adder --tdir picker_out/Adder --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/Adder/example.py picker_out/Adder/python/
elif [[ $@ == *"java"* ]]; then
    cp example/Adder/example.java picker_out/Adder/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/Adder/example.scala picker_out/Adder/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/Adder/example.go picker_out/Adder/golang/
elif [[ $@ == *"lua"* ]]; then
    cp example/Adder/example.lua picker_out/Adder/lua/
else
    cp example/Adder/example.cpp picker_out/Adder/cpp/
fi

cd picker_out/Adder && make EXAMPLE=ON
