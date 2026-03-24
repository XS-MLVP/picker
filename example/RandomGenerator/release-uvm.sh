#!/bin/bash

rm -rf picker_out_rmg/

./build/bin/picker pack --from-rtl example/RandomGenerator/RandomGenerator.v -d -e

rm -rf uvmpy/example.sv
rm -rf uvmpy/example.py

cp example/RandomGenerator/example.py uvmpy/
cp example/RandomGenerator/example-uvm.sv uvmpy/
cp example/RandomGenerator/RandomGenerator.v uvmpy/

mv uvmpy picker_out_rmg

cd picker_out_rmg && make
