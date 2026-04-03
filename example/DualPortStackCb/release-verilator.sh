#!/bin/bash

rm -rf picker_out/DualPortStackCb

./build/bin/picker export example/DualPortStackCb/dual_port_stack.v --autobuild false -w DualPortStackCb.fst $@ --tdir picker_out/DualPortStackCb
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    if [[ "$async" != "" ]]; then
        cp example/DualPortStackCb/example_async.py picker_out/DualPortStackCb/python/example.py
    else
        cp example/DualPortStackCb/example.py picker_out/DualPortStackCb/python/
    fi
elif [[ $@ == *"java"* ]]; then
    cp example/DualPortStackCb/example.java picker_out/DualPortStackCb/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/DualPortStackCb/example.scala picker_out/DualPortStackCb/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/DualPortStackCb/example.go picker_out/DualPortStackCb/golang/
else
    cp example/DualPortStackCb/example.cpp picker_out/DualPortStackCb/cpp/
fi

cd picker_out/DualPortStackCb && make EXAMPLE=ON
