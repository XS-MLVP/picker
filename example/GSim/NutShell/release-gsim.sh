#!/bin/bash


if ! command -v gsim &> /dev/null
then
    echo "gsim could not be found"
    echo "please add gsim into path first"
    exit
fi

rm -rf picker_out_GSIM/
if [ ! -f example/GSim/NutShell/SimTop-nutshell.fir ]; then
    tar -xf example/GSim/NutShell/SimTop-nutshell.tar.bz2 -C example/GSim/NutShell/
fi
./build/bin/picker export example/GSim/NutShell/SimTop-nutshell.fir --rw mem_direct --autobuild false --sim gsim --tdir picker_out_GSIM/NutShell $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/GSim/NutShell/example.py picker_out_GSIM/NutShell/python/
    cp example/GSim/NutShell/microbench-NutShell.bin picker_out_GSIM/NutShell/
elif [[ $@ == *"java"* ]]; then
    cp example/GSim/NutShell/example.java picker_out_GSIM/NutShell/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/GSim/NutShell/example.scala picker_out_GSIM/NutShell/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/GSim/NutShell/example.go picker_out_GSIM/NutShell/golang/
elif [[ $@ == *"lua"* ]]; then
    cp example/GSim/NutShell/example.lua picker_out_GSIM/NutShell/lua/
elif [[ $@ == *"cpp"* ]]; then
    cp example/GSim/NutShell/example.cpp picker_out_GSIM/NutShell/cpp/
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
cd picker_out_GSIM/NutShell && make EXAMPLE=ON
