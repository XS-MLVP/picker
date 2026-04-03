#!/bin/bash

rm -rf picker_out/RandomGenerator
./build/bin/picker export example/RandomGenerator/RandomGenerator.v --autobuild false --sim vcs -w RandomGenerator.fsdb --sname RandomGenerator --tdir picker_out/RandomGenerator --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/RandomGenerator/example.py picker_out/RandomGenerator/python/
elif [[ $@ == *"java"* ]]; then
    cp example/RandomGenerator/example.java picker_out/RandomGenerator/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/RandomGenerator/example.scala picker_out/RandomGenerator/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/RandomGenerator/example.go picker_out/RandomGenerator/golang/
else
    cp example/RandomGenerator/example.cpp picker_out/RandomGenerator/cpp/
fi

cd picker_out/RandomGenerator && make EXAMPLE=ON

if [[ $@ == *"python"* ]]; then
    echo "'cannot allocate memory in static TLS block'error for VCS is expected, please ignore it"
    LD_PRELOAD=./libUTRandomGenerator.so python3 example.py
fi
