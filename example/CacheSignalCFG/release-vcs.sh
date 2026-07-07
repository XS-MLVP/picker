#!/bin/bash
set -e

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="$(realpath -m "${OUT_ROOT}/CacheSignalCFG")"

rm -rf "$OUT_DIR"
./build/bin/picker export example/Cache/Cache.v --autobuild true --sim vcs -w Cache.fsdb --sname Cache --tdir "$OUT_DIR" --sdir template "$@"

if [[ $* == *"python"* ]]; then
    cp example/CacheSignalCFG/example.py "$OUT_DIR"
elif [[ $* == *"java"* ]]; then
    cp example/CacheSignalCFG/example.java "$OUT_DIR"
elif [[ $* == *"scala"* ]]; then
    cp example/CacheSignalCFG/example.scala "$OUT_DIR"
elif [[ $* == *"golang"* ]]; then
    cp example/CacheSignalCFG/example.go "$OUT_DIR"
elif [[ $* == *"lua"* ]]; then
    cp example/CacheSignalCFG/example.lua "$OUT_DIR"
else
    cp example/CacheSignalCFG/example.cpp "$OUT_DIR"
fi

cd "$OUT_DIR"

if [[ $* == *"python"* ]]; then
    python3 example.py
elif [[ $* == *"java"* ]]; then
    java -cp ./UT_Cache-java.jar:./xspcomm-java.jar -ea com.ut.example
elif [[ $* == *"scala"* ]]; then
    scala -cp ./UT_Cache-scala.jar:./xspcomm-scala.jar -ea com.ut.example
elif [[ $* == *"golang"* ]]; then
    GO111MODULE=off GOPATH="$(pwd)/golang" go build example.go
    ./example
elif [[ $* == *"lua"* ]]; then
    LUA_PATH="./xspcomm/?.lua" lua example.lua
else
    ./UTCache_example
fi
