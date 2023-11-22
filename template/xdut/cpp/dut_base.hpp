#pragma once
#include <bits/stdc++.h>
#include <svdpi.h>
#include "openvip/data.h"
#include "openvip/port.h"

using namespace ovip;

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
    virtual int step(uint64_t cycle, bool dump) = 0;
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
    void init(int, char**);
    DutVerilatorBase(int argc, char **argv);
    DutVerilatorBase();
    DutVerilatorBase(char *filename);
    DutVerilatorBase(char *filename, int argc, char **argv);
    DutVerilatorBase(std::initializer_list<const char *> args);
    ~DutVerilatorBase();
    int step();
    int step_nodump();
    int step(bool dump);
    int step(uint64_t cycle, bool dump);
    int finalize();
};

#endif

#if defined(USE_VCS)
#include "vc_hdrs.h"

extern "C" {
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
    void init(int, char**);
    DutVcsBase(int argc, char **argv);
    [[deprecated("VCS does not support no-args constructor")]] DutVcsBase();
    DutVcsBase(char *filename);
    DutVcsBase(char *filename, int argc, char **argv);
    DutVcsBase(std::initializer_list<const char *> args);
    ~DutVcsBase();
    int step();
    int step_nodump();
    int step(bool dump);
    int step(uint64_t cycle, bool dump);
    int finalize();
};

#endif
