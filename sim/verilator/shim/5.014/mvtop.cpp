#include "mvtop.h"

#include "Vhello.h"
#include "Vhello__Syms.h"

MVTop::MVTop(MVContext *contextp)
{
    this->contextp_ = contextp;
    this->topp_ = new Vhello((VerilatedContext *)(contextp->contextp_), "TOP");
}

MVTop::~MVTop()
{
    delete (Vhello *)(this->topp_);
}

void MVTop::eval_step()
{
    ((Vhello *)(this->topp_))->eval_step();
}

void MVTop::eval_end_step()
{
    ((Vhello *)(this->topp_))->eval_end_step();
}

void MVTop::final()
{
    ((Vhello *)(this->topp_))->final();
}

bool MVTop::eventsPending()
{
    return ((Vhello *)(this->topp_))->eventsPending();
}

u_int64_t MVTop::nextTimeSlot()
{
    return ((Vhello *)(this->topp_))->nextTimeSlot();
}

const char *MVTop::name() const
{
    return ((Vhello *)(this->topp_))->name();
}

const char *MVTop::hierName() const
{
    return ((Vhello *)(this->topp_))->hierName();
}

const char *MVTop::modelName() const
{
    return ((Vhello *)(this->topp_))->modelName();
}

unsigned MVTop::threads() const
{
    return ((Vhello *)(this->topp_))->threads();
}
