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

int DutVcsBase::Finish()
{
    // Finish VCS context
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
    // Warn("Shared DPI Library is required for Verilator");
    //  save argc and argv for debug
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
    // Finish Verilator context
    this->Finish();
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

int DutVerilatorBase::Finish()
{
    // Finish Verilator context
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
    DutVerilatorBase *res = new DutVerilatorBase(argc, argv);
};
typedef DutVerilatorBase *dlcreates_t(int argc, char **argv);

void dlstep(DutVerilatorBase *dut, uint64_t ncycle, bool dump)
{
    dut->Step(ncycle, dump);
};
typedef void step_t(DutVerilatorBase *, uint64_t, bool);

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


int DutUnifiedBase::lib_count = 0;
bool DutUnifiedBase::main_ns_flag = false;

DutUnifiedBase::DutUnifiedBase()
{
#if defined(USE_VERILATOR)
    // find the lib.so file path which contains this class
    char *argv[] = {locateLibPath()};
    this->init(1, argv);
#elif defined(USE_VCS)
    Fatal("VCS does not support no-args constructor");
#endif
    free(argv[0]);
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
    free(name);
};
DutUnifiedBase::DutUnifiedBase(char *filename, int argc, char **argv)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    argv[0] = name;
    this->init(argc, argv);
    free(name);
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
    for (int i = 0; i < argc; i++) { free(argv[i]); }
};
void DutUnifiedBase::init(int argc, char **argv)
{
    // hold argc and argv for later use
    this->argc = argc;
    this->argv = (char **)malloc(sizeof(char *) * argc);
    for (size_t i = 0; i < argc; i++)
    {
        this->argv[i] = (char *)malloc(strlen(argv[i]) + 1);
        strcpy(this->argv[i], argv[i]);
    } 

    // the main namespace instance doesn't need to load the shared library
    if (!main_ns_flag) {
        Info("Using main namespace");
#if defined(USE_VERILATOR)
        this->dut = new DutVerilatorBase(this->argc, this->argv);
#elif defined(USE_VCS)
        this->dut = new DutVcsBase(this->argc, this->argv);
#endif
        main_ns_flag = true;
        lib_handle   = nullptr;
        return;
    }
    // get dynamic library path from argv
    if (argc == 0) {
        Fatal("Shared DPI Library Path is required for Verilator");
    }

    this->lib_handle = dlmopen(LM_ID_NEWLM, this->argv[0], RTLD_NOW | RTLD_DEEPBIND);
    if (!this->lib_handle) {
        Fatal("Failed to open shared DPI library %s, %s", argv[0], dlerror());
    }
    this->lib_count++;

    // create top module
    dlcreates_t *dlcreates =
        (dlcreates_t *)dlsym(this->lib_handle, "dlcreates");
    if (!dlcreates) { Fatal("Failed to find dlcreates function"); }
    this->dut = dlcreates(this->argc, this->argv);
}
uint64_t DutUnifiedBase::GetDPIHandle(char *name, int towards)
{
    char *func_name = (char *)malloc(strlen(name) + 5);
    if (towards == 0) {
        sprintf(func_name, "get_%sxx{{__LIB_DPI_FUNC_NAME_HASH__}}", name);
    } else if (towards == 1) {
        sprintf(func_name, "set_%sxx{{__LIB_DPI_FUNC_NAME_HASH__}}", name);
    } else if (towards == -1) {
        strcpy(func_name, name);
    } else {
        Fatal("Invalid DPI function request %s %d", name, towards);
    }

    void *func;
    if (this->lib_handle != nullptr) {
        func = dlsym(this->lib_handle, func_name);
    } else {
        func = dlsym(RTLD_DEFAULT, func_name);
    }

    // internal only support read
    if (func == nullptr && towards == 0) {
        Fatal("Failed to find DPI function %s", func_name);
    }

    return (uint64_t)func;
}

int DutUnifiedBase::simStep()
{
    return this->simStep(1, 1);
}
int DutUnifiedBase::stepNoDump()
{
    return this->simStep(1, 0);
}
int DutUnifiedBase::simStep(bool dump)
{
    return this->simStep(1, dump);
}
int DutUnifiedBase::simStep(uint64_t cycle, bool dump)
{
    // if (this->lib_handle == nullptr) {
    //     return this->dut->Step(cycle, dump);
    // }
    // step_t *simStep = (step_t *)dlsym(this->lib_handle, "dlstep");
    // if (!simStep) {
    //     Fatal("Failed to find simStep function");
    // }
    // simStep(this->dut, cycle, dump);
    return this->dut->Step(cycle, dump);
}
int DutUnifiedBase::RefreshComb()
{
    return this->simStep(1, 0);
}
int DutUnifiedBase::Finish()
{
    if (!this->dut) {
        return 0;
    }
    this->dut->Finish();
    delete this->dut;
    // this class maintain the other namespace
    if (this->lib_handle != nullptr) {
        dlclose(this->lib_handle);
        this->lib_count--;
    } else { // this class is using the main namespace
        this->main_ns_flag = false;
    }
    for (int i = 0; i < this->argc; i++) { free(this->argv[i]); }\
    return 0;
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
    // Clean up the new instance with the shared library
    this->Finish();
    
}
