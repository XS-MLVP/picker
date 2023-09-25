#include "mvtop.h"

#include "Vtop.h"
#include "Vtop__Syms.h"

MVTop::MVTop(MVContext *contextp) :
    contextp_(contextp),
    topp_(new Vtop((VerilatedContext *)(contextp->contextp_), "TOP")),
    clk(((Vtop *)(this->topp_))->clk)
{
}

MVTop::~MVTop()
{
    delete (Vtop *)(this->topp_);
}

void MVTop::eval_step()
{
    ((Vtop *)(this->topp_))->eval_step();
}

void MVTop::eval_end_step()
{
    ((Vtop *)(this->topp_))->eval_end_step();
}

void MVTop::final()
{
    ((Vtop *)(this->topp_))->final();
}

bool MVTop::eventsPending()
{
    return ((Vtop *)(this->topp_))->eventsPending();
}

u_int64_t MVTop::nextTimeSlot()
{
    return ((Vtop *)(this->topp_))->nextTimeSlot();
}

const char *MVTop::name() const
{
    return ((Vtop *)(this->topp_))->name();
}

const char *MVTop::hierName() const
{
    return ((Vtop *)(this->topp_))->hierName();
}

const char *MVTop::modelName() const
{
    return ((Vtop *)(this->topp_))->modelName();
}

unsigned MVTop::threads() const
{
    return ((Vtop *)(this->topp_))->threads();
}

void MVTop::trace(MVWaveC *tfp, int levels, int options = 0)
{
    ((Vtop *)(this->topp_))->trace((VerilatedVcdC *)(tfp->tracep_), levels, options);
}
