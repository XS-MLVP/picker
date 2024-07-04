#include "dut_base.hpp"
#include <dlfcn.h>

DutBase::DutBase()
{
    cycle = 0;
    argc  = 0;
    argv  = nullptr;
}

#if defined(USE_VCS)
#include "vc_hdrs.h"
DutVcsBase::DutVcsBase()
{
    // FATAL("VCS does not support no-args constructor");
    exit(-1);
}

DutVcsBase::DutVcsBase(int argc, char **argv)
{
    // save argc and argv for debug
    this->argc = argc;
    this->argv = argv;
    this->init(argc, argv);
};

void DutVcsBase::init(int argc, char **argv)
{
    // initialize VCS context
    VcsMain(argc, argv);

    // set VCS cycle to 0
    VcsInit();
    svSetScope(svGetScopeFromName("{{__TOP_MODULE_NAME__}}_top"));

    // set cycle pointer to 0
    this->cycle               = 0;
    this->cycle_hl            = 0;
    this->vcs_clock_period[0] = {{__VCS_CLOCK_PERIOD_HIGH__}};
    this->vcs_clock_period[1] = {{__VCS_CLOCK_PERIOD_LOW__}};
    this->vcs_clock_period[2] = {{__VCS_CLOCK_PERIOD_LOW__}}
                                + {{__VCS_CLOCK_PERIOD_HIGH__}};
}

DutVcsBase::~DutVcsBase(){};

int DutVcsBase::Step()
{
    return DutVcsBase::Step(1, 1);
};

int DutVcsBase::StepNoDump()
{
    assert(0);
    return 0;
};

int DutVcsBase::Step(bool dump)
{
    return DutVcsBase::Step(dump, dump);
};

int DutVcsBase::Step(uint64_t ncycle, bool dump)
{
    if (!dump) {
        assert(ncycle == 0);
        VcsSimUntil(&cycle);
        return 0;
    }

    // set cycle pointer
    cycle_hl += ncycle;
    if (likely(ncycle == 1))
        cycle += vcs_clock_period[cycle_hl & 1];
    else
        cycle += (ncycle >> 1) * vcs_clock_period[2]
                 + (ncycle & 1) * vcs_clock_period[cycle_hl & 1];

    // run simulation
    VcsSimUntil(&cycle);
    return 0;
};

int DutVcsBase::Finished()
{
    // Finished VCS context
    return 0;
};

#endif

#if defined(USE_VERILATOR)
#include "verilated.h"
#include "V{{__TOP_MODULE_NAME__}}.h"

#if defined(VL_TRACE)
#include "V{{__TOP_MODULE_NAME__}}___024root.h"
#include "V{{__TOP_MODULE_NAME__}}__Syms.h"
#endif

DutVerilatorBase::DutVerilatorBase()
{
    this->init(0, nullptr);
}

DutVerilatorBase::DutVerilatorBase(int argc, char **argv)
{
    //Warn("Shared DPI Library is required for Verilator");
    // save argc and argv for debug
    this->argc = argc;
    this->argv = argv;
    this->init(argc, argv);
};

void DutVerilatorBase::init(int argc, char **argv)
{
    // save argc and argv for debug
    this->argc = argc;
    this->argv = argv;

    // create top module
    VerilatedContext *contextp = new VerilatedContext;
    contextp->randReset(2);
    contextp->debug(0);
    contextp->commandArgs(argc, argv);

#if defined(VL_TRACE)
    contextp->traceEverOn(true);
#endif

    // create top module
    this->top = new V{{__TOP_MODULE_NAME__}} {contextp};

    svSetScope(svGetScopeFromName("TOP.{{__TOP_MODULE_NAME__}}_top"));

    // set cycle pointer to 0
    this->cycle = 0;
};

DutVerilatorBase::~DutVerilatorBase()
{
    // Finished Verilator context
    this->Finished();
};

int DutVerilatorBase::Step()
{
    // push one more cycle
    return DutVerilatorBase::Step(1, 1);
};

int DutVerilatorBase::StepNoDump()
{
    return DutVerilatorBase::Step(1, 0);
};

int DutVerilatorBase::Step(bool dump)
{
    return DutVerilatorBase::Step(1, dump);
};

int DutVerilatorBase::Step(uint64_t ncycle, bool dump)
{
    cycle += ncycle;
    if (dump) {
        for (int i = 0; i < ncycle; i++) {
            ((V{{__TOP_MODULE_NAME__}} *)(top))->eval();
            ((V{{__TOP_MODULE_NAME__}} *)(top))->contextp()->timeInc(1);
        }
    } else {
        assert(ncycle == 1);
        ((V{{__TOP_MODULE_NAME__}} *)(top))->eval_step();
    }

    return 0;
};

int DutVerilatorBase::Finished()
{
    // Finished Verilator context
    if (this->top != nullptr) {
#if defined(VL_COVERAGE)
        VerilatedContext *contextp =
            ((V{{__TOP_MODULE_NAME__}} *)(this->top))->contextp();
        if (this->coverage_file_path.size() > 0)
            contextp->coveragep()->write(this->coverage_file_path.c_str());
        else
            contextp->coveragep()->write(
                "V{{__TOP_MODULE_NAME__}}_coverage.dat");
#endif
        ((V{{__TOP_MODULE_NAME__}} *)(this->top))->final();
        delete (V{{__TOP_MODULE_NAME__}} *)(this->top);
        this->top = nullptr;
    }
    return 0;
};

void DutVerilatorBase::SetWaveform(const char *filename)
{
#if defined(VL_TRACE)
    ((V{{__TOP_MODULE_NAME__}} *)(this->top))->contextp()->dumpfile(filename);
    ((V{{__TOP_MODULE_NAME__}} *)(this->top))->rootp->vlSymsp->_traceDumpOpen();
#else
    std::cerr << "Verilator waveform is not enabled";
    exit(-1);
#endif
};

void DutVerilatorBase::SetCoverage(const char *filename)
{
#if defined(VL_COVERAGE)
    this->coverage_file_path = filename;
#else
    std::cerr << "Verilator coverage is not enabled";
    exit(-1);
#endif
};

DutVerilatorBase *dlcreates(int argc, char **argv)
{
    DutVerilatorBase* res = new DutVerilatorBase(argc, argv);
};
typedef DutVerilatorBase *dlcreates_t(int argc, char **argv);

#endif

char *locateLibPath()
{
    Dl_info info;
    if (dladdr((char *)locateLibPath, &info) == 0) {
        Fatal("Failed to find the shared library path");
    }
    std::string lib_path = info.dli_fname;
    Info("Shared DPI Library Path: %s", lib_path.c_str());
    std::string lib_name = lib_path.substr(lib_path.find_last_of("/") + 1);

    char *res = (char *)malloc(lib_name.size() + 1);
    strcpy(res, lib_name.c_str());
    return res;
}

DutUnifiedBase::DutUnifiedBase()
{
#if defined(USE_VERILATOR)
    // find the lib.so file path which contains this class
    char *argv[] = {locateLibPath()};
    this->init(1, argv);
#elif defined(USE_VCS)
    Fatal("VCS does not support no-args constructor");
#endif
}

DutUnifiedBase::DutUnifiedBase(int argc, char **argv)
{
    this->init(argc, argv);
}
DutUnifiedBase::DutUnifiedBase(char *filename)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    char *argv[] = {name};
    this->init(1, argv);
};
DutUnifiedBase::DutUnifiedBase(char *filename, int argc, char **argv)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    argv[0] = name;
    this->init(argc, argv);
};
DutUnifiedBase::DutUnifiedBase(std::initializer_list<const char *> args)
{
    int argc    = args.size();
    char **argv = (char **)malloc(sizeof(char *) * (argc + 128));
    memset(argv, -1, sizeof(char *) * (argc + 128));
    int i = 0;
    for (auto arg : args) {
        char *name = (char *)malloc(strlen(arg) + 1);
        strcpy(name, arg);
        argv[i++] = name;
    }
    this->init(argc, argv);
};
void DutUnifiedBase::init(int argc, char **argv)
{
#if defined(USE_VERILATOR)
    // get dynamic library path from argv
    if (argc == 0) {
        Fatal("Shared DPI Library Path is required for Verilator");
    }
    this->lib_handle = dlmopen(LM_ID_NEWLM, argv[0], RTLD_NOW | RTLD_DEEPBIND);
    if (!this->lib_handle) {
        Fatal("Failed to open shared DPI library %s, %s", argv[0], dlerror());
    }

    // create top module
    dlcreates_t *dlcreates =
        (dlcreates_t *)dlsym(this->lib_handle, "dlcreates");
    if (!dlcreates) { Fatal("Failed to find dlcreates function"); }
    this->dut = dlcreates(argc, argv);

#elif defined(USE_VCS)
    this->dut = new DutVcsBase(argc, argv);
#endif
}
uint64_t DutUnifiedBase::GetDPIHandle(char *name, int towards)
{
    char *func_name = (char *)malloc(strlen(name) + 5);
    if (towards == 0) {
        sprintf(func_name, "get_%s", name);
    } else if (towards == 1) {
        sprintf(func_name, "set_%s", name);
    } else {
        Fatal("Invalid DPI function request %s %d", name, towards);
    }

    void *func = dlsym(this->lib_handle, func_name);
    if (func == nullptr) { Fatal("Failed to find DPI function %s", func_name); }

    return (uint64_t)func;
}

int DutUnifiedBase::DStep()
{
    return this->dut->Step();
}
int DutUnifiedBase::DStepNoDump()
{
    return this->dut->StepNoDump();
}
int DutUnifiedBase::DStep(bool dump)
{
    return this->dut->Step(dump);
}
int DutUnifiedBase::DStep(uint64_t cycle, bool dump)
{
    return this->dut->Step(cycle, dump);
}

int DutUnifiedBase::Finished()
{
    return this->dut->Finished();
}
void DutUnifiedBase::SetWaveform(const char *filename)
{
    return this->dut->SetWaveform(filename);
}
void DutUnifiedBase::SetCoverage(const char *filename)
{
    return this->dut->SetCoverage(filename);
}

DutUnifiedBase::~DutUnifiedBase()
{
    this->Finished();
    for (int i = 0; i < this->argc; i++) { free(this->argv[i]); }
}
