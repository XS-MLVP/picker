#include "dut_base.hpp"

DutBase::DutBase()
{
    cycle = 0;
    argc = 0;
    argv = nullptr;
}

#if defined(USE_VCS)

DutVcsBase::DutVcsBase(int argc, char **argv)
{
    // save argc and argv for debug
    this->argc = argc;
    this->argv = argv;

    // initialize VCS context
    VcsMain(argc, argv);

    // set VCS cycle to 0
    VcsInit();
    svSetScope(svGetScopeFromName("{{__TOP_MODULE_NAME__}}_top"));

    // set cycle pointer to 0
    this->cycle = 0;
    this->to_cycle[0] = 0;
};

DutVcsBase::~DutVcsBase(){};

int DutVcsBase::step()
{
    // push one more cycle
    return DutVcsBase::step(1);
};

int DutVcsBase::step(uint64_t cycle)
{
    // set cycle pointer
    this->cycle += cycle;
    this->to_cycle[1] = this->cycle;

    // run simulation
    VcsSimUntil(this->to_cycle);
    return 0;
};

int DutVcsBase::finalize()
{
    // finalize VCS context
    return 0;
};

#endif

#if defined(USE_VERILATOR)

DutVerilatorBase::DutVerilatorBase(int argc, char **argv)
{
    // save argc and argv for debug
    this->argc = argc;
    this->argv = argv;

    // initialize Verilator context
    Verilated::commandArgs(argc, argv);

    // create top module
    VerilatedContext *contextp = new VerilatedContext;
    contextp->randReset(2);
    contextp->debug(0);
    contextp->commandArgs(argc, argv);
    this->top = new V{{__TOP_MODULE_NAME__}} {contextp};

#if defined(VL_TRACE)
    contextp->traceEverOn(true);
#endif

    svSetScope(svGetScopeFromName("TOP.{{__TOP_MODULE_NAME__}}_top"));

    // set cycle pointer to 0
    this->cycle = 0;
};

DutVerilatorBase::~DutVerilatorBase()
{
    // finalize Verilator context
    delete (V{{__TOP_MODULE_NAME__}} *)(this->top);
};

int DutVerilatorBase::step()
{
    // push one more cycle
    return DutVerilatorBase::step(1);
};

int DutVerilatorBase::step(uint64_t cycle)
{
    // set cycle pointer
    this->cycle += cycle;

    // run simulation
    for (int i = 0; i < cycle; i++)
        ((V{{__TOP_MODULE_NAME__}} *)(this->top))->eval();
    return 0;
};

int DutVerilatorBase::finalize()
{
    // finalize Verilator context
    return 0;
};

#endif
