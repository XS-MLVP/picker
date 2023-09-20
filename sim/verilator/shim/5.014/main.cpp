#include "mvtop.h"
int main(int argc, char **argv)
{
    MVContext *contextp = new MVContext;
    contextp->commandArgs(argc, argv);
    MVTop *top = new MVTop{contextp};
    while (!contextp->gotFinish())
    {
        top->eval();
    }
    delete top;
    delete contextp;
 
    return 0;
}