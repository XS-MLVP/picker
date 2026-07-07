#!/bin/bash

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/e203_ifu_ift2icb"

rm -rf "$OUT_DIR"
./build/bin/picker export --fs example/e203_ifu_ift2icb/filelist.txt --autobuild false -w e203_ifu_ift2icb.fst --sname e203_ifu_ift2icb --tdir "$OUT_DIR" -V "--no-timing" --sdir template "$@"
# if python in $@, then it will generate python binding
if [[ $* == *"python"* ]]; then
    cp example/e203_ifu_ift2icb/example.py "$OUT_DIR/python/"
elif [[ $* == *"java"* ]]; then
    cp example/e203_ifu_ift2icb/example.java "$OUT_DIR/java/"
elif [[ $* == *"scala"* ]]; then
    cp example/e203_ifu_ift2icb/example.scala "$OUT_DIR/scala/"
elif [[ $* == *"golang"* ]]; then
    cp example/e203_ifu_ift2icb/example.go "$OUT_DIR/golang/"
elif [[ $* == *"lua"* ]]; then
    cp example/e203_ifu_ift2icb/example.lua "$OUT_DIR/lua/"
else
    cp example/e203_ifu_ift2icb/example.cpp "$OUT_DIR/cpp/"
fi

cd "$OUT_DIR" && make EXAMPLE=ON
