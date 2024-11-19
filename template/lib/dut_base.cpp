#include "dut_base.hpp"
#include <dlfcn.h>
#include <unistd.h>
#include <string>
#include <cstdlib>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

bool enable_xinfo = true;

DutBase::DutBase()
{
    cycle = 0;
    argc  = 0;
    argv  = nullptr;
}

#if defined(USE_VCS)

DutVcsBase::DutVcsBase()
{
    // XXFatal("VCS does not support no-args constructor");
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

DutVcsBase::~DutVcsBase() {};

int DutVcsBase::Step(uint64_t ncycle, bool dump)
{
    if (!dump) {
        // assert(ncycle == 0);
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
    finish_{{__LIB_DPI_FUNC_NAME_HASH__}}();
    return 0;
};

void DutVcsBase::SetWaveform(const char *filename)
{
    XInfo("VCS waveform is not supported");
};
void DutVcsBase::SetCoverage(const char *filename)
{
    XInfo("VCS coverage is not supported");
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
        VerilatedContext *contextp =
            ((V{{__TOP_MODULE_NAME__}} *)(this->top))->contextp();
#if defined(VL_COVERAGE)
        if (this->coverage_file_path.size() > 0)
            contextp->coveragep()->write(this->coverage_file_path.c_str());
        else
            contextp->coveragep()->write(
                "V{{__TOP_MODULE_NAME__}}_coverage.dat");
#endif
        ((V{{__TOP_MODULE_NAME__}} *)(this->top))->final();
        delete (V{{__TOP_MODULE_NAME__}} *)(this->top);
        delete contextp;
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
    return res;
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
        XFatal("Failed to find the shared library path");
    }

    std::string lib_path = info.dli_fname;
    XInfo("Shared DPI Library Path: %s", lib_path.c_str());

    // get PWD
    char *pwd = get_current_dir_name();

    // get relative path
    std::string rel_path;
    if (lib_path.find(pwd) == std::string::npos) {
        rel_path = lib_path;
    } else {
        rel_path = lib_path.substr(strlen(pwd) + 1);
    }

    char *res = (char *)malloc(rel_path.size() + 128);
    strcpy(res, rel_path.c_str());
    return res;
}

inline void vcsLibPathConvert(char *path)
{
    // locate 'libUT' and replace it with 'libDPI' for VCS
    // move the other char to next position
    char *p  = path;
    int plib = 0, pend = strlen(path) + 1;
    plib = strstr(path, "libUT") - p;
    while (pend > plib) {
        p[pend + 1] = p[pend];
        pend--;
    }
    strncpy(p + plib, "libDPI", 6);
    XInfo("vcsLibPath %s", path);
}

int DutUnifiedBase::lib_count     = 0;
bool DutUnifiedBase::main_ns_flag = false;

DutUnifiedBase::DutUnifiedBase()
{
    this->init(0, nullptr);
}

DutUnifiedBase::DutUnifiedBase(int argc, char **argv)
{
    this->init(argc, (const char**)argv);
}

DutUnifiedBase::DutUnifiedBase(char *filename)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    char *argv[] = {name};
    this->init(1, (const char**)argv);
    free(name);
};

DutUnifiedBase::DutUnifiedBase(char *filename, int argc, char **argv)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    argv[0] = name;
    this->init(argc, (const char**)argv);
    free(name);
};

DutUnifiedBase::DutUnifiedBase(std::initializer_list<const char *> args)
{
    int argc    = 0;
    const char **argv = (const char **)malloc(sizeof(char *) * argc);
    for (auto arg : args) { argv[argc++] = arg; }
    this->init(argc, argv);
    free(argv);
};

DutUnifiedBase::DutUnifiedBase(std::vector<std::string> args)
{
    int argc    = 0;
    const char **argv = (const char **)malloc(sizeof(char *) * argc);
    for (auto arg : args) { 
        argv[argc] = (char *)malloc(arg.size() + 1);
        strcpy((char *)argv[argc], arg.c_str());
        argc++;
    }
    this->init(argc, argv);
    for (int i = 0; i < argc; i++) { 
        XInfo("Initial Args %d: %s", i, argv[i]);
        free((char *)argv[i]); 
    }
    free(argv);
}

void DutUnifiedBase::init(int argc, const char **argv)
{
    // check whether the ENABLE_XINFO is set
    const char *enable_xinfo_env = std::getenv("ENABLE_XINFO");
    if (enable_xinfo_env) {
        auto value = std::string(enable_xinfo_env);
        if (value == "0") {
            enable_xinfo = false;
        }
        if (value == "1") {
            enable_xinfo = true;
        }
    }
    // hold argc and argv for later use
    this->argc = argc;
    this->argv = (char **)malloc(sizeof(char *) * (argc + 128));
    memset(this->argv, -1, sizeof(char *) * (argc + 128));
    for (size_t i = 0; i < argc; i++) {
        this->argv[i] = (char *)malloc(strlen(argv[i]) + 1);
        strcpy(this->argv[i], argv[i]);
    }

    // find whether the shared library path is provided
    if (argc == 0 || !std::string(this->argv[0]).ends_with(".so")) {
        // add the shared library path to argv
        for (int i = argc; i > 0; i--) { this->argv[i] = this->argv[i - 1]; }
        this->argv[0] = locateLibPath();
#if defined(USE_VCS)
        vcsLibPathConvert(this->argv[0]);
#endif
        this->argc++;
    }


    // the main namespace instance doesn't need to load the shared library
    if (!main_ns_flag) {
        XInfo("Using main namespace");
#if defined(USE_VERILATOR)
        this->dut = new DutVerilatorBase(this->argc, this->argv);
#elif defined(USE_VCS)
        this->dut = new DutVcsBase(this->argc, this->argv);
#endif
        main_ns_flag = true;
        lib_handle   = nullptr;
        return;
    }

#ifndef USE_VCS
    // get dynamic library path from argv
    if (this->argc == 0) {
        XFatal("Shared DPI Library Path is required for Simulator");
    }

    this->lib_handle =
        dlmopen(LM_ID_NEWLM, this->argv[0], RTLD_NOW | RTLD_DEEPBIND);
    if (!this->lib_handle) {
        XFatal("Failed to open shared DPI library %s, %s", this->argv[0], dlerror());
    }
    this->lib_count++;

    // create top module
    dlcreates_t *dlcreates =
        (dlcreates_t *)dlsym(this->lib_handle, "dlcreates");
    if (!dlcreates) { XFatal("Failed to find dlcreates function"); }
    this->dut = dlcreates(this->argc, this->argv);
#endif
}

uint64_t DutUnifiedBase::GetDPIHandle(std::string name, int towards)
{
    return this->GetDPIHandle((char *)name.c_str(), towards);
}

uint64_t DutUnifiedBase::GetDPIHandle(char *name, int towards)
{
    char *func_name = (char *)malloc(strlen(name) + 128);
    if (towards == 0) {
        sprintf(func_name, "get_%sxx{{__LIB_DPI_FUNC_NAME_HASH__}}", name);
    } else if (towards == 1) {
        sprintf(func_name, "set_%sxx{{__LIB_DPI_FUNC_NAME_HASH__}}", name);
    } else if (towards == -1) {
        strcpy(func_name, name);
    } else {
        XFatal("Invalid DPI function request %s %d", name, towards);
    }

    void *func;
    if (this->lib_handle != nullptr) {
        func = dlsym(this->lib_handle, func_name);
    } else {
        func = dlsym(RTLD_DEFAULT, func_name);
    }

    // internal only support read
    if (func == nullptr && towards == 0) {
        XFatal("Failed to find DPI function %s", func_name);
    }
    free(func_name);
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
    //     XFatal("Failed to find simStep function");
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
    if (!this->dut) { return 0; }
    this->dut->Finish();
    delete this->dut;
    this->dut = nullptr;
    // this class maintain the other namespace
    if (this->lib_handle != nullptr) {
        dlclose(this->lib_handle);
        this->lib_count--;
    } else { // this class is using the main namespace
        this->main_ns_flag = false;
    }
    for (int i = 0; i < this->argc; i++) { free(this->argv[i]); }
    return 0;
}
void DutUnifiedBase::SetWaveform(const char *filename)
{
    return this->dut->SetWaveform(filename);
}
void DutUnifiedBase::SetCoverage(const std::string filename)
{
    return this->dut->SetCoverage(filename.c_str());
}
void DutUnifiedBase::SetCoverage(const char *filename)
{
    return this->dut->SetCoverage(filename);
}
void DutUnifiedBase::SetWaveform(const std::string filename)
{
    return this->dut->SetWaveform(filename.c_str());
}

DutUnifiedBase::~DutUnifiedBase()
{
    // Clean up the new instance with the shared library
    this->Finish();
}
