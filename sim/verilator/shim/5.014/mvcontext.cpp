#include <verilated.h>
#include "mvcontext.h"

MVContext::MVContext()
{
    this->contextp_ = new VerilatedContext();
}

MVContext::~MVContext()
{
    delete (VerilatedContext *)this->contextp_;
}

void MVContext::commandArgs(int argc, const char **argv)
{
    ((VerilatedContext *)(this->contextp_))->commandArgs(argc, argv);
}

void MVContext::commandArgs(int argc, char **argv)
{
    ((VerilatedContext *)(this->contextp_))->commandArgs(argc, const_cast<const char **>(argv));
}

void MVContext::commandArgsAdd(int argc, const char **argv)
{
    ((VerilatedContext *)(this->contextp_))->commandArgsAdd(argc, argv);
}

void MVContext::gotError(bool flag)
{
    ((VerilatedContext *)(this->contextp_))->gotError(flag);
}

bool MVContext::gotError()
{
    return ((VerilatedContext *)(this->contextp_))->gotError();
}

void MVContext::gotFinish(bool flag)
{
    ((VerilatedContext *)(this->contextp_))->gotFinish(flag);
}

bool MVContext::gotFinish() const
{
    return ((VerilatedContext *)(this->contextp_))->gotFinish();
}

void MVContext::randReset(int val)
{
    ((VerilatedContext *)(this->contextp_))->randReset(val);
}

int MVContext::randReset()
{
    return ((VerilatedContext *)(this->contextp_))->randReset();
}

void MVContext::randSeed(int val)
{
    ((VerilatedContext *)(this->contextp_))->randSeed(val);
}

int MVContext::randSeed() const
{
    return ((VerilatedContext *)(this->contextp_))->randSeed();
}
