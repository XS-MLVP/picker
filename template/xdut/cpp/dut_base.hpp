#pragma once
#include <bits/stdc++.h>
#include <svdpi.h>
#include "data.hpp"

class DutBase
{
public:
    u_int64_t cycle;
    int argc;
    char **argv;

    // Initialize
    DutBase();
    virtual ~DutBase() = default;

    // Simulate 1 cycle
    virtual int step() = 0;

    // Simulate N cycles
    virtual int step(uint64_t cycle) = 0;
    // Clean up and dump result
    virtual int finalize() = 0;
};

#if defined(USE_VERILATOR)

#include "verilated.h"
#include "V{{__TOP_MODULE_NAME__}}.h"

class DutVerilatorBase : public DutBase
{
public:
    // Verilator Context and Top Module
    void *top;
    DutVerilatorBase(int argc, char **argv);
    ~DutVerilatorBase();
    int step();
    int step(uint64_t cycle);
    int finalize();
};

#endif

#if defined(USE_VCS)
#include "vc_hdrs.h"

extern "C"
{
    int VcsMain(int argc, char **argv);
    void VcsInit();
    void VcsSimUntil(int *);
}

class DutVcsBase : public DutBase
{
protected:
    // VCS SimUntil
    int to_cycle[2] = {0, 0};

public:
    DutVcsBase(int argc, char **argv);
    ~DutVcsBase();
    int step();
    int step(uint64_t cycle);
    int finalize();
};

#endif
