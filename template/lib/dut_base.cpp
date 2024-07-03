#include "dut_base.hpp"
#include "xspcomm/xutil.h"

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

DutVcsBase::DutVcsBase(char *filename)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    char *argv[] = {name};
    this->init(1, argv);
};

DutVcsBase::DutVcsBase(char *filename, int argc, char **argv)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    argv[0] = name;
    this->init(argc, argv);
};

DutVcsBase::DutVcsBase(std::initializer_list<const char *> args)
{
    int argc    = args.size();
    // Reserve heap space for VCS overflow ?
    char **argv = (char **)malloc(sizeof(char *) * (argc+128));
    memset(argv, -1, sizeof(char *) * (argc+128));
    int i       = 0;
    for (auto arg : args) {
        char *name = (char *)malloc(strlen(arg) + 1);
        strcpy(name, arg);
        argv[i++] = name;
    }
    this->init(argc, argv);
};

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
    this->cycle    = 0;
    this->cycle_hl = 0;
    this->vcs_clock_period[0] = {{__VCS_CLOCK_PERIOD_HIGH__}};
    this->vcs_clock_period[1] = {{__VCS_CLOCK_PERIOD_LOW__}};
    this->vcs_clock_period[2] = {{__VCS_CLOCK_PERIOD_LOW__}} + {{__VCS_CLOCK_PERIOD_HIGH__}};
}

DutVcsBase::~DutVcsBase(){};

int DutVcsBase::step()
{
    return DutVcsBase::step(1, 1);
};

int DutVcsBase::step_nodump()
{
    assert(0);
    return 0;
};

int DutVcsBase::step(bool dump)
{
    return DutVcsBase::step(dump, dump);
};

int DutVcsBase::step(uint64_t ncycle, bool dump)
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

int DutVcsBase::finished()
{
    // finished VCS context
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
    Warn("No-args constructor is not supported for Verilator");
}

DutVerilatorBase::DutVerilatorBase(char *filename)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    char *argv[] = {name};
    this->init(1, argv);
};

DutVerilatorBase::DutVerilatorBase(char *filename, int argc, char **argv)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    argv[0] = name;
    this->init(argc, argv);
};

DutVerilatorBase::DutVerilatorBase(std::initializer_list<const char *> args)
{
    int argc    = args.size();
    char **argv = (char **)malloc(sizeof(char *) * argc);
    int i       = 0;
    for (auto arg : args) {
        char *name = (char *)malloc(strlen(arg) + 1);
        strcpy(name, arg);
        argv[i++] = name;
    }
    this->init(argc, argv);
};

DutVerilatorBase::DutVerilatorBase(int argc, char **argv)
{
    Warn("Shared DPI Library is required for Verilator");
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

    // load DPI library
    this->lib_handle = dlmopen(LM_ID_NEWLM, "libDPI{{__TOP_MODULE_NAME__}}.so",  RTLD_NOW | RTLD_DEEPBIND);
    if (!this->lib_handle) {
        Fatal("Failed to load DPI library: %s", dlerror());
    }
    Debug("Loaded DPI library: %s", "libDPI{{__TOP_MODULE_NAME__}}.so");
    Lmid_t lmid;
    dlinfo(this->lib_handle, RTLD_DI_LMID, &lmid);

    // create top module
    VerilatedContext *contextp = new VerilatedContext;
    contextp->randReset(2);
    contextp->debug(0);
    contextp->commandArgs(argc, argv);
    
    // due to dlmopen, the verilator context is in another namespace
    // so we need to use dlsym to get the function pointer
    typedef svScope (*svSetScopeFunc)(svScope);
    typedef svScope (*svGetScopeFromNameFunc)(const char*);
    typedef V{{__TOP_MODULE_NAME__}}* (*dlcreatesFunc)(VerilatedContext* vc);

    dlcreatesFunc _dlcreates = reinterpret_cast<dlcreatesFunc>(dlsym(this->lib_handle, "dlcreates"));
    if (!_dlcreates) {
        Fatal("Failed to load dlcreates function");
    }
    this->top = _dlcreates(contextp);

#if defined(VL_TRACE)
    contextp->traceEverOn(true);
#endif
    
    // set simulation scope
    svSetScopeFunc _svSetScope = reinterpret_cast<svSetScopeFunc>(dlsym(this->lib_handle, "svSetScope"));
    svGetScopeFromNameFunc _svGetScopeFromName = reinterpret_cast<svGetScopeFromNameFunc>(dlsym(this->lib_handle, "svGetScopeFromName"));
    if (!_svSetScope || !_svGetScopeFromName ) {
        std::cerr << "Failed to load simulation scope functions." << std::endl;
        return;
    }
    _svSetScope(_svGetScopeFromName("TOP.{{__TOP_MODULE_NAME__}}_top"));

    // set cycle pointer to 0
    this->cycle = 0;
};

DutVerilatorBase::~DutVerilatorBase()
{
    // finished Verilator context
    this->finished();
};

int DutVerilatorBase::step()
{
    // push one more cycle
    return DutVerilatorBase::step(1, 1);
};

int DutVerilatorBase::step_nodump()
{
    return DutVerilatorBase::step(1, 0);
};

int DutVerilatorBase::step(bool dump)
{
    return DutVerilatorBase::step(1, dump);
};

int DutVerilatorBase::step(uint64_t ncycle, bool dump)
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

int DutVerilatorBase::finished()
{
    // finished Verilator context
    if (this->top != nullptr) {
#if defined(VL_COVERAGE)
        VerilatedContext *contextp = ((V{{__TOP_MODULE_NAME__}} *)(this->top))->contextp();
        if (this->coverage_file_path.size() > 0)
            contextp->coveragep()->write(this->coverage_file_path.c_str());
        else
            contextp->coveragep()->write("V{{__TOP_MODULE_NAME__}}_coverage.dat");
#endif
        ((V{{__TOP_MODULE_NAME__}} *)(this->top))->final();
        delete (V{{__TOP_MODULE_NAME__}} *)(this->top);
        this->top = nullptr;
    }
    return 0;
};

uint64_t DutVerilatorBase::get_dpi_handle(char *name, int towards)
{
    char *func_name = (char *)malloc(strlen(name) + 5);
    if (!func_name) {
        Fatal("Memory allocation failed for func_name");
    }
    if (towards == 0) { // 0 is read from sv
        strcpy(func_name, "get_");
    } else if (towards == 1) { // 1 is write to sv
        strcpy(func_name, "set_");
    } else {
        free(func_name);
        Fatal("towards should be 0 or 1");
    }
    strcat(func_name, name);
    uint64_t handle = (uint64_t)dlsym(this->lib_handle, func_name);
    if (!handle) {
        Fatal("Failed to load DPI function: %s", func_name);
    }
    free(func_name);
    return handle;
};

void DutVerilatorBase::set_waveform(const char *filename)
{
#if defined(VL_TRACE)
    ((V{{__TOP_MODULE_NAME__}} *)(this->top))->contextp()->dumpfile(filename);
    ((V{{__TOP_MODULE_NAME__}} *)(this->top))->rootp->vlSymsp->_traceDumpOpen();
#else
    std::cerr << "Verilator waveform is not enabled";
    exit(-1);
#endif
};

void DutVerilatorBase::set_coverage(const char *filename)
{
#if defined(VL_COVERAGE)
    this->coverage_file_path = filename;
#else
    std::cerr << "Verilator coverage is not enabled";
    exit(-1);
#endif
};


#endif


#if defined(USE_VERILATOR)
DutUnifiedBase::DutUnifiedBase() : DutVerilatorBase() {};
DutUnifiedBase::DutUnifiedBase(int argc, char **argv) : DutVerilatorBase(argc, argv){};
DutUnifiedBase::DutUnifiedBase(char *filename) : DutVerilatorBase(filename) {};
DutUnifiedBase::DutUnifiedBase(char *filename, int argc, char **argv) : DutVerilatorBase(filename, argc, argv) {};
DutUnifiedBase::DutUnifiedBase(std::initializer_list<const char *> args) : DutVerilatorBase(args) {};
int DutUnifiedBase::finished() {
    return DutVerilatorBase::finished();
}
uint64_t DutUnifiedBase::get_dpi_handle(char *name, int towards) {
    return DutVerilatorBase::get_dpi_handle(name, towards);
}
void DutUnifiedBase::set_waveform(const char *filename) {
    return DutVerilatorBase::set_waveform(filename);
}
void DutUnifiedBase::set_coverage(const char *filename) {
    return DutVerilatorBase::set_coverage(filename);
}
#elif defined(USE_VCS)
DutUnifiedBase::DutUnifiedBase() : DutVcsBase() {};
DutUnifiedBase::DutUnifiedBase(int argc, char **argv) : DutVcsBase(argc, argv){};
DutUnifiedBase::DutUnifiedBase(char *filename) : DutVcsBase(filename) {};
DutUnifiedBase::DutUnifiedBase(char *filename, int argc, char **argv) : DutVcsBase(filename, argc, argv) {};
DutUnifiedBase::DutUnifiedBase(std::initializer_list<const char *> args) : DutVcsBase(args) {};
int DutUnifiedBase::finished() {
    return DutVcsBase::finished();
}
void DutUnifiedBase::set_waveform(const char *filename) {
    return DutVcsBase::set_waveform(filename);
}
void DutUnifiedBase::set_coverage(const char *filename) {
    return DutVcsBase::set_coverage(filename);
}
#endif

DutUnifiedBase::~DutUnifiedBase()
{
    // finished {{__TOP_MODULE_NAME__}}
}
