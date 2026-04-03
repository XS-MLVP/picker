#!/bin/bash

rm -rf picker_out/RandomGenerator

./build/bin/picker export example/RandomGenerator/RandomGenerator.v --autobuild false -w RandomGenerator.fst $@ --tdir picker_out/RandomGenerator
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
