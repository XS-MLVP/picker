#pragma once
#include <bits/stdc++.h>
#include <svdpi.h>

typedef struct {
    uint64_t addr;
    int width;
} native_signal_t;

class DutBase
{
public:
    uint64_t cycle;
    int argc;
    char **argv;
    std::map<std::string, native_signal_t> native_signals;
    bool native_signal_enable = false;

    // Initialize
    DutBase();
    virtual ~DutBase() = default;

    // Simulate N cycles
    virtual int Step(uint64_t cycle, bool dump) = 0;
    // Clean up and dump result
    virtual int Finish() = 0;

    // Set waveform file path
    virtual void SetWaveform(const char *filename) = 0;
    virtual void FlushWaveform()                   = 0;
    virtual void WaveformEnable(bool enable)       = 0;
    // Set coverage file path
    virtual void SetCoverage(const char *filename) = 0;
    // Save Model Status with Simulator Capabilities
    virtual int CheckPoint(const char *filename) = 0;
    // Load Model Status with Simulator Capabilities
    virtual int Restore(const char *filename) = 0;
    // Has native signal support
    virtual bool NativeSignalEnabled();
    virtual uint64_t NativeSignalAddr(std::string &name);
    virtual native_signal_t NativeSignalGet(std::string &name);
    virtual int NativeSignalWidth(std::string &name);
    virtual std::vector<std::string> NativeSignalList(std::string &name, int depth);
};

#if defined(USE_VERILATOR)
class DutVerilatorBase : public DutBase
{
private:
    std::string coverage_file_path;

public:
    // Verilator Context and Top Module
    std::string sv_scope = "TOP.{{__TOP_MODULE_NAME__}}_top";
    void *top;
    void init(int, char **);
    DutVerilatorBase();
    DutVerilatorBase(int argc, char **argv);
    ~DutVerilatorBase();
    int Step(uint64_t cycle, bool dump);
    int Finish();
    void SetWaveform(const char *filename);
    void FlushWaveform();
    void WaveformEnable(bool enable);
    void SetCoverage(const char *filename);
    int CheckPoint(const char *filename);
    int Restore(const char *filename);
};
extern "C" {
DutVerilatorBase *dlcreates(int argc, char **argv);
void dlstep(DutVerilatorBase *dut, uint64_t ncycle, bool dump);
}
#endif

#if defined(USE_VCS)
extern "C" {
int VcsMain(int argc, char **argv);
void VcsInit();
void VcsSimUntil(uint64_t *);
void finish_{{__LIB_DPI_FUNC_NAME_HASH__}}();
}
#include "vc_hdrs.h"

class DutVcsBase : public DutBase
{
protected:
    uint64_t cycle_hl;
    uint64_t vcs_clock_period[3];

public:
    std::string sv_scope = "{{__TOP_MODULE_NAME__}}_top";
    void init(int, char **);
    DutVcsBase(int argc, char **argv);
    [[deprecated("VCS does not support no-args constructor")]] DutVcsBase();
    ~DutVcsBase();
    int Step(uint64_t cycle, bool dump);
    int Finish();
    void SetWaveform(const char *filename);
    void FlushWaveform();
    void WaveformEnable(bool enable);
    void SetCoverage(const char *filename);
    int CheckPoint(const char *filename);
    int Restore(const char *filename);
};

#endif

char *locateLibPath();

class DutUnifiedBase
{
protected:
    static int lib_count;
    static bool main_ns_flag; // is there any instance of DutUnifiedBase in main namespace
    void *lib_handle;
    int argc;
    char **argv;

#if defined(USE_VERILATOR)
    DutVerilatorBase *dut;
#elif defined(USE_VCS)
    DutVcsBase *dut;
#endif

public:
    // Multi-arg constructor
    DutUnifiedBase();
    DutUnifiedBase(int argc, char **argv);
    DutUnifiedBase(char *filename);
    DutUnifiedBase(char *filename, int argc, char **argv);
    DutUnifiedBase(std::initializer_list<const char *> args);
    DutUnifiedBase(std::vector<std::string> args);

    // Destructor
    ~DutUnifiedBase();

    // Call simulator instance to initialize
    void init(int, const char **);

    // Do simulate
    int simStep(bool dump); // Simulate 1 cycle
    int simStep(uint64_t cycle, bool dump);
    int RefreshComb(); // Simulate, but time doesn't pass

    // Do simulate with xcomm lib
    uint64_t pSelf = (uint64_t)this;
    uint64_t pxcStep = (uint64_t)&xcommStep;
    static int xcommStep(uint64_t base_ptr, uint64_t cycle, bool dump);

    // Clean up and dump result
    int Finish();

    // Get DPI handle for XData
    uint64_t GetDPIHandle(char *name, int towards);
    uint64_t GetDPIHandle(std::string name, int towards);

    // Get VPI function pointer to use VPI functions
    uint64_t GetVPIFuncPtr(const char *name);
    uint64_t GetVPIFuncPtr(std::string name);

    // Get VPI handle object to apply VPI operations
    uint64_t GetVPIHandleObj(const char *name);
    uint64_t GetVPIHandleObj(std::string name);

    // Iterate all internal signals
    std::vector<std::string> VPIInternalSignalList(char *name, int depth);
    std::vector<std::string> VPIInternalSignalList(std::string name, int depth);

    // Set waveform file path
    void SetWaveform(const char *filename);
    void SetWaveform(const std::string filename);

    // Flush waveform from mem cache to file
    void FlushWaveform();

    // Set whether waveform is logged to file or not (time still pass)
    void WaveformEnable(bool enable = true);

    // Set coverage file path
    void SetCoverage(const char *filename);      
    void SetCoverage(const std::string filename); 

    // Save Model Status with Simulator Capabilities
    // Return current cycle, warning: strictly limited to same design
    int CheckPoint(const char *filename);
    int CheckPoint(const std::string filename);

    // Load Model Status with Simulator Capabilities
    // Return current cycle, warning: strictly limited to same design
    int Restore(const char *filename);
    int Restore(const std::string filename);

    // Native signal support
    bool NativeSignalEnabled();
    native_signal_t NativeSignalGet(std::string name);
    int NativeSignalWidth(std::string name);
    uint64_t NativeSignalAddr(std::string name);
    std::vector<std::string> NativeSignalList(std::string name, int depth=99);
};

extern int enable_xinfo;

#define XFatal(fmt, ...)                                                                                               \
    do {                                                                                                               \
        fprintf(stderr, "FATAL: " fmt, ##__VA_ARGS__);                                                                 \
        puts("");                                                                                                      \
        exit(-1);                                                                                                      \
    } while (0)

#define XInfo(fmt, ...)                                                                                                \
    do {                                                                                                               \
        if (enable_xinfo) {                                                                                            \
            fprintf(stdout, "INFO: " fmt, ##__VA_ARGS__);                                                              \
            puts("");                                                                                                  \
        }                                                                                                              \
    } while (0)

#define XDebug(fmt, ...)                                                                                               \
    do {                                                                                                               \
        if (enable_xinfo > 1) {                                                                                        \
            fprintf(stdout, "DEBUG: " fmt, ##__VA_ARGS__);                                                             \
            puts("");                                                                                                  \
        }                                                                                                              \
    } while (0)

#define XWarning(fmt, ...)                                                                                             \
    do {                                                                                                               \
        fprintf(stderr, "WARNING: " fmt, ##__VA_ARGS__);                                                               \
        puts("");                                                                                                      \
    } while (0)
