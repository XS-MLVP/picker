#!/bin/bash

rm -rf picker_out/e203_ifu_ift2icb
./build/bin/picker export --fs example/e203_ifu_ift2icb/filelist.txt --autobuild false -w e203_ifu_ift2icb.fst --sname e203_ifu_ift2icb --tdir picker_out/e203_ifu_ift2icb -V "--no-timing" --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/e203_ifu_ift2icb/example.py picker_out/e203_ifu_ift2icb/python/
elif [[ $@ == *"java"* ]]; then
    cp example/e203_ifu_ift2icb/example.java picker_out/e203_ifu_ift2icb/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/e203_ifu_ift2icb/example.scala picker_out/e203_ifu_ift2icb/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/e203_ifu_ift2icb/example.go picker_out/e203_ifu_ift2icb/golang/
elif [[ $@ == *"lua"* ]]; then
    cp example/e203_ifu_ift2icb/example.lua picker_out/e203_ifu_ift2icb/lua/
else
    cp example/e203_ifu_ift2icb/example.cpp picker_out/e203_ifu_ift2icb/cpp/
fi

cd picker_out/e203_ifu_ift2icb && make EXAMPLE=ON
