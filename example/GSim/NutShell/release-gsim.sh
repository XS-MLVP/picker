#!/bin/bash


if ! command -v gsim &> /dev/null
then
    echo "gsim could not be found"
    echo "please add gsim into path first"
    exit
fi

OUT_ROOT="${OUT_ROOT:-output}"
OUT_DIR="${OUT_ROOT}/NutShell"

rm -rf "$OUT_DIR"
if [ ! -f example/GSim/NutShell/SimTop-nutshell.fir ]; then
    tar -xf example/GSim/NutShell/SimTop-nutshell.tar.bz2 -C example/GSim/NutShell/
fi
./build/bin/picker export example/GSim/NutShell/SimTop-nutshell.fir --rw mem_direct --autobuild false --sim gsim --tdir "$OUT_DIR" "$@"
# if python in $@, then it will generate python binding
if [[ $* == *"python"* ]]; then
    cp example/GSim/NutShell/example.py "$OUT_DIR/python/"
    cp example/GSim/NutShell/microbench-NutShell.bin "$OUT_DIR/"
elif [[ $* == *"java"* ]]; then
    cp example/GSim/NutShell/example.java "$OUT_DIR/java/"
elif [[ $* == *"scala"* ]]; then
    cp example/GSim/NutShell/example.scala "$OUT_DIR/scala/"
elif [[ $* == *"golang"* ]]; then
    cp example/GSim/NutShell/example.go "$OUT_DIR/golang/"
elif [[ $* == *"lua"* ]]; then
    cp example/GSim/NutShell/example.lua "$OUT_DIR/lua/"
elif [[ $* == *"cpp"* ]]; then
    cp example/GSim/NutShell/example.cpp "$OUT_DIR/cpp/"
else
    echo "No example file copied, use default config."
fi

if [ -z $CC ] && [ -z $CXX ]; then
    echo "CC and CXX not set, using default clang and clang++."
    export CC=clang
    export CXX=clang++
else
    echo "Using CC=$CC and CXX=$CXX"
fi
cd "$OUT_DIR" && make EXAMPLE=ON
