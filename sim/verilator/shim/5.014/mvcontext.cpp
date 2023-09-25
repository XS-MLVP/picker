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

uint64_t MVContext::time() const
{
    return ((VerilatedContext *)(this->contextp_))->time();
}

void MVContext::time(uint64_t value)
{
    ((VerilatedContext *)(this->contextp_))->time(value);
}

/// @brief NOT Multi-thread safe! Use only when single threaded.
/// @param add 
void MVContext::timeInc(uint64_t add)
{
    ((VerilatedContext *)(this->contextp_))->timeInc(add);
}

int MVContext::timeunit() const
{
    return ((VerilatedContext *)(this->contextp_))->timeunit();
}

void MVContext::timeunit(int value)
{
    ((VerilatedContext *)(this->contextp_))->timeunit(value);
}

const char *MVContext::timeunitString() const
{
    return ((VerilatedContext *)(this->contextp_))->timeunitString();
}

int MVContext::timeprecision() const
{
    return ((VerilatedContext *)(this->contextp_))->timeprecision();
}

void MVContext::timeprecision(int value)
{
    ((VerilatedContext *)(this->contextp_))->timeprecision(value);
}

const char *MVContext::timeprecisionString() const
{
    return ((VerilatedContext *)(this->contextp_))->timeprecisionString();
}

unsigned MVContext::threads() const
{
    return ((VerilatedContext *)(this->contextp_))->threads();
}

void MVContext::threads(unsigned value)
{
    ((VerilatedContext *)(this->contextp_))->threads(value);
}

void MVContext::traceEverOn(bool flag)
{
    ((VerilatedContext *)(this->contextp_))->traceEverOn(flag);
}