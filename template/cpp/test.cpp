#include <verilated.h>
#include "{{dut_name}}.h"

int Test_{{dut_name}}(int argc, char** argv) {
    VerilatedContext* contextp = new VerilatedContext;
    contextp->commandArgs(argc, argv);
    {{dut_name}}* top = new {{dut_name}}{contextp};
    while (!contextp->gotFinish()) {
        top->eval();
    }
    top->final();
    delete top;
    return 0;
}

int main(int argc, char** argv){
    Test_{{dut_name}}(argc,argv);
}
