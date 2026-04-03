#!/bin/bash

rm -rf picker_out/AdderUVM

./build/bin/picker pack --from-rtl example/Adder/Adder.v -n Adder -d -e

rm -rf uvmpy/example.sv
rm -rf uvmpy/example.py

cp example/Adder/example.py uvmpy/
cp example/Adder/example-uvm.sv uvmpy/

mv uvmpy picker_out/AdderUVM

cd picker_out/AdderUVM && make
