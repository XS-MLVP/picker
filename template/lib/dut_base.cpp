#include "dut_base.hpp"
#include <dlfcn.h>
#include <unistd.h>
#include <string>
#include <cstdlib>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

int enable_xinfo = 0; // 0: disable, 1: enable, 2: debug

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
    svSetScope(svGetScopeFromName(this->sv_scope.c_str()));

    // set cycle pointer to 0
    this->cycle               = 0;
    this->cycle_hl            = 0;
    this->vcs_clock_period[0] = {{__VCS_CLOCK_PERIOD_HIGH__}};
    this->vcs_clock_period[1] = {{__VCS_CLOCK_PERIOD_LOW__}};
    this->vcs_clock_period[2] = {{__VCS_CLOCK_PERIOD_LOW__}} + {{__VCS_CLOCK_PERIOD_HIGH__}};
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
        cycle += (ncycle >> 1) * vcs_clock_period[2] + (ncycle & 1) * vcs_clock_period[cycle_hl & 1];

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
void DutVcsBase::FlushWaveform()
{
    XInfo("VCS waveform is not supported");
};
void DutVcsBase::WaveformEnable(bool enable)
{
    XInfo("VCS waveform is not supported");
};
int DutVcsBase::CheckPoint(const char *filename)
{
    XFatal("VCS checkpoint is not supported");
};
int DutVcsBase::Restore(const char *filename)
{
    XFatal("VCS restore is not supported");
};

#endif

#if defined(USE_VERILATOR)
#include "verilated.h"
#include "V{{__TOP_MODULE_NAME__}}.h"

#if defined(VL_TRACE)
#include "V{{__TOP_MODULE_NAME__}}___024root.h"
#include "V{{__TOP_MODULE_NAME__}}__Syms.h"
#endif

#if defined(VL_VPI)
#include "verilated_vpi.h"
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

    svSetScope(svGetScopeFromName(this->sv_scope.c_str()));

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
#if defined(VL_VPI)
    VerilatedVpi::callValueCbs();
#endif
    if (dump) {
        for (int i = 0; i < ncycle; i++) {
            ((V{{__TOP_MODULE_NAME__}} *)(top))->eval();
#if defined(VL_VPI)
            VerilatedVpi::callValueCbs(); 
#endif
            ((V{{__TOP_MODULE_NAME__}} *)(top))->contextp()->timeInc(1);
        }
    } else {
        assert(ncycle == 1);
        ((V{{__TOP_MODULE_NAME__}} *)(top))->eval_step();
    }
#if defined(VL_VPI)
    VerilatedVpi::callValueCbs(); 
#endif

    return 0;
};

int DutVerilatorBase::Finish()
{
    // Finish Verilator context
    if (this->top != nullptr) {
        VerilatedContext *contextp = ((V{{__TOP_MODULE_NAME__}} *)(this->top))->contextp();
#if defined(VL_COVERAGE)
        if (this->coverage_file_path.size() > 0)
            contextp->coveragep()->write(this->coverage_file_path.c_str());
        else
            contextp->coveragep()->write("V{{__TOP_MODULE_NAME__}}_coverage.dat");
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

void DutVerilatorBase::FlushWaveform()
{
#if defined(VL_TRACE)
    V{{__TOP_MODULE_NAME__}} *topp = (V{{__TOP_MODULE_NAME__}} *)(this->top);
    topp->rootp->vlSymsp->__Vm_dumperp->flush();
#else
    std::cerr << "Verilator waveform is not enabled";
    exit(-1);
#endif
};
void DutVerilatorBase::WaveformEnable(bool enable=true)
{
#if defined(VL_TRACE)
    V{{__TOP_MODULE_NAME__}} *topp = (V{{__TOP_MODULE_NAME__}} *)(this->top);
    topp->rootp->vlSymsp->__Vm_dumping = enable;
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

#if defined(VL_SAVEABLE)
#include "verilated_save.h"
int DutVerilatorBase::CheckPoint(const char *filename)
{
    VerilatedSave os;
    os.open(filename);
    os << this->cycle;
    os << *(V{{__TOP_MODULE_NAME__}} *)(top);
    return this->cycle;
};

int DutVerilatorBase::Restore(const char *filename)
{
    VerilatedRestore os;
    os.open(filename);
    os >> this->cycle;
    os >> *(V{{__TOP_MODULE_NAME__}} *)(top);
    return this->cycle;
};
#else
int DutVerilatorBase::CheckPoint(const char *filename)
{
    XFatal("Verilator checkpoint is not enabled");
};

int DutVerilatorBase::Restore(const char *filename)
{
    XFatal("Verilator restore is not enabled");
};
#endif

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
    if (dladdr((char *)locateLibPath, &info) == 0) { XFatal("Failed to find the shared library path"); }

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
    this->init(argc, (const char **)argv);
}

DutUnifiedBase::DutUnifiedBase(char *filename)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    char *argv[] = {name};
    this->init(1, (const char **)argv);
    free(name);
};

DutUnifiedBase::DutUnifiedBase(char *filename, int argc, char **argv)
{
    char *name = (char *)malloc(strlen(filename) + 1);
    strcpy(name, filename);
    argv[0] = name;
    this->init(argc, (const char **)argv);
    free(name);
};

DutUnifiedBase::DutUnifiedBase(std::initializer_list<const char *> args)
{
    int argc          = 0;
    const char **argv = (const char **)malloc(sizeof(char *) * argc);
    for (auto arg : args) { argv[argc++] = arg; }
    this->init(argc, argv);
    free(argv);
};

DutUnifiedBase::DutUnifiedBase(std::vector<std::string> args)
{
    int argc          = 0;
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
        if (value == "1") {
            enable_xinfo = 1;
        } else if (value == "2") {
            enable_xinfo = 2;
        }
    }
    // hold argc and argv for later use
    this->argc = argc;
    this->argv = (char **)malloc(sizeof(char *) * (argc + 128));
    memset(this->argv, -1, sizeof(char *) * (argc + 128));
    for (size_t i = 0; i < argc; i++) {
        auto tsize    = strlen(argv[i]) + 32;
        this->argv[i] = (char *)malloc(tsize);
        memset(this->argv[i], 0, tsize);
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
    if (this->argc == 0) { XFatal("Shared DPI Library Path is required for Simulator"); }

    this->lib_handle = dlmopen(LM_ID_NEWLM, this->argv[0], RTLD_NOW | RTLD_DEEPBIND);
    if (!this->lib_handle) { XFatal("Failed to open shared DPI library %s, %s", this->argv[0], dlerror()); }
    this->lib_count++;

    // create top module
    dlcreates_t *dlcreates = (dlcreates_t *)dlsym(this->lib_handle, "dlcreates");
    if (!dlcreates) { XFatal("Failed to find dlcreates function"); }
    this->dut = dlcreates(this->argc, this->argv);
#endif
}

uint64_t DutUnifiedBase::GetDPIHandle(std::string name, int towards)
{
    return this->GetDPIHandle((char *)name.c_str(), towards);
}

uint64_t DutUnifiedBase::GetVPIFuncPtr(const char *name)
{
    return this->GetVPIFuncPtr(std::string(name));
}

uint64_t DutUnifiedBase::GetVPIFuncPtr(std::string name)
{
    void *func;
    if (this->lib_handle != nullptr) {
        func = dlsym(this->lib_handle, name.c_str());
    } else {
        func = dlsym(RTLD_DEFAULT, name.c_str());
    }
    if (func == nullptr) { XInfo("Failed to find VPI function %s", name.c_str()); }
    return (uint64_t)func;
}

uint64_t DutUnifiedBase::GetVPIHandleObj(const char *name)
{
    return this->GetVPIHandleObj(std::string(name));
}

uint64_t DutUnifiedBase::GetVPIHandleObj(std::string name)
{
    uint64_t _get_vpi_handle_name = this->GetVPIFuncPtr("vpi_handle_by_name");
    std::string scope             = name.size() > 0 ? this->dut->sv_scope + "." + name : this->dut->sv_scope;
    uint64_t vpi_handle           = ((uint64_t(*)(char *, uint32_t))_get_vpi_handle_name)((char *)scope.c_str(), 0);
    return vpi_handle;
}

std::vector<std::string> DutUnifiedBase::VPIInternalSignalList(char *name, int depth)
{
    return VPIInternalSignalList(std::string(name), depth);
}

std::vector<std::string> DutUnifiedBase::VPIInternalSignalList(std::string name, int depth)
{
    std::vector<std::string> res;
    std::string scope = name.size() > 0 ? this->dut->sv_scope + "." + name : this->dut->sv_scope;

    // Define the VPI functions
    typedef uint64_t vpi_handle_t;
    typedef vpi_handle_t (*vpi_handle_by_name_t)(char *, vpi_handle_t);
    typedef vpi_handle_t (*vpi_get_t)(uint32_t, vpi_handle_t);
    typedef vpi_handle_t (*vpi_get_str_t)(uint32_t, vpi_handle_t);
    typedef vpi_handle_t (*vpi_scan_t)(vpi_handle_t);
    typedef vpi_handle_t (*vpi_iterate_t)(uint32_t, vpi_handle_t);
    typedef vpi_handle_t (*vpi_handle_func)(uint32_t, vpi_handle_t);

#define vpiType 1          /* type of object */
#define vpiName 2          /* local name of object */
#define vpiFullName 3      /* full hierarchical name */
#define vpiModule 32       /* module instance */
#define vpiModuleArray 112 /* module instance array */
#define vpiNet 36          /* scalar or vector net */
#define vpiNetArray 114    /* multidimensional net */
#define vpiReg 48          /* scalar or vector reg */
#define vpiRegArray 116    /* multidimensional reg */

    // Get the VPI functions
    vpi_handle_by_name_t _get_vpi_handle_name = (vpi_handle_by_name_t)this->GetVPIFuncPtr("vpi_handle_by_name");
    vpi_get_t _vpi_get                        = (vpi_get_t)this->GetVPIFuncPtr("vpi_get");
    vpi_get_str_t _vpi_get_str                = (vpi_get_str_t)this->GetVPIFuncPtr("vpi_get_str");
    vpi_scan_t _vpi_scan                      = (vpi_scan_t)this->GetVPIFuncPtr("vpi_scan");
    vpi_iterate_t _vpi_iterate                = (vpi_iterate_t)this->GetVPIFuncPtr("vpi_iterate");
    vpi_handle_func _vpi_handle               = (vpi_handle_func)this->GetVPIFuncPtr("vpi_handle");

    // Remove extra "TOP" while input name is empty(TOP)
    std::function<std::string(std::string)> remove_top = [&](std::string sig) {
#ifdef USE_VERILATOR
        return sig.substr(4);
#else
        return sig;
#endif
    };

    // Define the lambda function to traverse the VPI handle
    std::function<void(vpi_handle_t, int)> traverse = [&](vpi_handle_t handle, int depth) {
        if (depth == 0) { return; }
        vpi_handle_t vpi_handle = 0;

        // get vpi reg
        vpi_handle_t regs = _vpi_iterate(vpiReg, handle);
        while ((vpi_handle = _vpi_scan(regs)) != 0) {
            char *name = (char *)_vpi_get_str(vpiFullName, vpi_handle);
            XDebug("Found reg %s", name);
            res.push_back(remove_top(name));
        }

        // get vpi reg array
        vpi_handle_t reg_arrays = _vpi_iterate(vpiRegArray, handle);
        while ((vpi_handle = _vpi_scan(reg_arrays)) != 0) {
            char *name = (char *)_vpi_get_str(vpiFullName, vpi_handle);
            XDebug("Found reg array %s", name);
            res.push_back(remove_top(name));
        }

        // get vpi net
        vpi_handle_t nets = _vpi_iterate(vpiNet, handle);
        while ((vpi_handle = _vpi_scan(nets)) != 0) {
            char *name = (char *)_vpi_get_str(vpiFullName, vpi_handle);
            XDebug("Found net %s", name);
            res.push_back(remove_top(name));
        }

        // get vpi net array
        vpi_handle_t net_arrays = _vpi_iterate(vpiNetArray, handle);
        while ((vpi_handle = _vpi_scan(net_arrays)) != 0) {
            char *name = (char *)_vpi_get_str(vpiFullName, vpi_handle);
            XDebug("Found net array %s", name);
            res.push_back(remove_top(name));
        }

        // get vpi module
        vpi_handle_t modules = _vpi_iterate(vpiModule, handle);
        while ((vpi_handle = _vpi_scan(modules)) != 0) {
            char *name = (char *)_vpi_get_str(vpiFullName, vpi_handle);
            XDebug("Found module %s", name);
            traverse(vpi_handle, depth - 1);
        }

        // get vpi module array
        vpi_handle_t module_arrays = _vpi_iterate(vpiModuleArray, handle);
        while ((vpi_handle = _vpi_scan(module_arrays)) != 0) {
            char *name = (char *)_vpi_get_str(vpiFullName, vpi_handle);
            XDebug("Found module array %s", name);
            traverse(vpi_handle, depth - 1);
        }
    };

    // Start iterating the initial scope
    vpi_handle_t vpi_handle = _get_vpi_handle_name((char *)scope.c_str(), 0);
    XDebug("Traversing %s %d", scope.c_str(), depth);
    if (vpi_handle == 0) { XInfo("Failed to find VPI handle %s", scope.c_str()); return res; }
    XDebug("Found VPI handle 0x%lx, type %ld", vpi_handle, _vpi_get(vpiType, vpi_handle));

    // Traverse the VPI handle
    traverse(vpi_handle, depth);

    return res;
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
    if (func == nullptr && towards == 0) { XFatal("Failed to find DPI function %s", func_name); }
    free(func_name);
    return (uint64_t)func;
}
int DutUnifiedBase::simStep(bool dump)
{
    return this->simStep(1, dump);
}
int DutUnifiedBase::simStep(uint64_t cycle, bool dump)
{
    return this->dut->Step(cycle, dump);
}
int DutUnifiedBase::xcommStep(uint64_t base_ptr, uint64_t cycle, bool dump)
{
    DutUnifiedBase *dut = (DutUnifiedBase *)base_ptr;
    return dut->simStep(cycle, dump);
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
void DutUnifiedBase::FlushWaveform()
{
    return this->dut->FlushWaveform();
}
void DutUnifiedBase::WaveformEnable(bool enable)
{
    return this->dut->WaveformEnable(enable);
}
int DutUnifiedBase::CheckPoint(const char *filename)
{
    return this->dut->CheckPoint(filename);
}

int DutUnifiedBase::CheckPoint(const std::string filename)
{
    return this->dut->CheckPoint(filename.c_str());
}

int DutUnifiedBase::Restore(const char *filename)
{
    return this->dut->Restore(filename);
}

int DutUnifiedBase::Restore(const std::string filename)
{
    return this->dut->Restore(filename.c_str());
}

DutUnifiedBase::~DutUnifiedBase()
{
    // Clean up the new instance with the shared library
    this->Finish();
}
