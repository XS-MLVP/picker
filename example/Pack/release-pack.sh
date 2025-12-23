#!/bin/bash
set -e

ROOT=$(pwd)
PACK_OUT="$ROOT/picker_out_pack"

prepare() {
    rm -rf "$PACK_OUT"
    mkdir -p "$PACK_OUT" && cd "$PACK_OUT"
}

test_basic() {
    prepare 
    "$ROOT/build/bin/picker" pack ../example/Pack/adder_trans.sv -e
    cd uvmpy/
    make clean comp copy_xspcomm run
}

test_send() {
    prepare 
    cp "$ROOT/example/Pack/Makefile" .
    "$ROOT/build/bin/picker" pack ../example/Pack/adder_trans.sv
    cp "$ROOT/example/Pack/01_Py2UVM/example.py" .
    cp "$ROOT/example/Pack/01_Py2UVM/example.sv" .
    make clean comp copy_xspcomm run
}

test_recv() {
    prepare 
    cp "$ROOT/example/Pack/Makefile" .
    "$ROOT/build/bin/picker" pack ../example/Pack/adder_trans.sv
    cp "$ROOT/example/Pack/02_UVM2Py/example.py" .
    cp "$ROOT/example/Pack/02_UVM2Py/example.sv" .
    make clean comp copy_xspcomm run
}

test_dut() {
    prepare 
    "$ROOT/build/bin/picker" pack ../example/Pack/adder_trans.sv -d -e
    mv uvmpy/* .
    rm -rf uvmpy/
    make clean comp copy_xspcomm run
}

test_multi() {
    prepare 
    cp "$ROOT/example/Pack/03_MultiTrans"/{alu_op.sv,alu_result.sv,filelist.txt} .
    cp "$ROOT/example/Pack/Makefile" .
    cp "$ROOT/example/Pack/03_MultiTrans/example.py" .
    cp "$ROOT/example/Pack/03_MultiTrans/example.sv" .
    "$ROOT/build/bin/picker" pack -f filelist.txt -n ALU -d
    make clean comp copy_xspcomm run
}

case "${1:-basic}" in
    dut)   test_dut   ;;
    multi) test_multi ;;
    recv)  test_recv  ;;
    send)  test_send  ;;
    all)   test_dut && test_multi && test_recv && test_send && test_adder ;;
    *)     test_basic; exit 1 ;;
esac
