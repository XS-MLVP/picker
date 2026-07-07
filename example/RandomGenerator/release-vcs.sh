#!/bin/bash
set -e

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="$(realpath -m "${OUT_ROOT}/RandomGenerator")"

rm -rf "$OUT_DIR"
./build/bin/picker export example/RandomGenerator/RandomGenerator.v --autobuild true --sim vcs -w RandomGenerator.fsdb --sname RandomGenerator --tdir "$OUT_DIR" --sdir template --coverage "$@"
# if python in $@, then it will generate python binding
if [[ $* == *"python"* ]]; then
    cp example/RandomGenerator/example.py "$OUT_DIR"
elif [[ $* == *"java"* ]]; then
    cp example/RandomGenerator/example.java "$OUT_DIR"
elif [[ $* == *"scala"* ]]; then
    cp example/RandomGenerator/example.scala "$OUT_DIR"
elif [[ $* == *"golang"* ]]; then
    cp example/RandomGenerator/example.go "$OUT_DIR"
elif [[ $* == *"lua"* ]]; then
    cp example/RandomGenerator/example.lua "$OUT_DIR"
else
    cp example/RandomGenerator/example.cpp "$OUT_DIR"
fi

cd "$OUT_DIR"

if [[ $* == *"python"* ]]; then
    python3 example.py
elif [[ $* == *"java"* ]]; then
    java -cp ./UT_RandomGenerator-java.jar:./xspcomm-java.jar -ea com.ut.example
elif [[ $* == *"scala"* ]]; then
    scala -cp ./UT_RandomGenerator-scala.jar:./xspcomm-scala.jar -ea com.ut.example
elif [[ $* == *"golang"* ]]; then
    GO111MODULE=off GOPATH="$(pwd)/golang" go build example.go
    ./example
elif [[ $* == *"lua"* ]]; then
    LUA_PATH="./xspcomm/?.lua" lua example.lua
else
    ./UTRandomGenerator_example
fi
