#pragma once
#include <bits/stdc++.h>
#include <svdpi.h>

class DutBase
{
public:
    uint64_t cycle;
    int argc;
    char **argv;

    // Initialize
    DutBase();
    virtual ~DutBase() = default;

    // Simulate N cycles
    virtual int Step(uint64_t cycle, bool dump) = 0;
    // Clean up and dump result
    virtual int Finish() = 0;

    // Set waveform file path
    virtual void SetWaveform(const char *filename) = 0;
    virtual void FlushWaveform() = 0;
    virtual void WaveformEnable(bool enable) = 0;
    // Set coverage file path
    virtual void SetCoverage(const char *filename) = 0;
    // Save Model Status with Simulator Capabilities
    virtual int CheckPoint(const char *filename) = 0;
    // Load Model Status with Simulator Capabilities
    virtual int Restore(const char *filename) = 0;
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
    DutUnifiedBase();
    DutUnifiedBase(int argc, char **argv);
    DutUnifiedBase(char *filename);
    DutUnifiedBase(char *filename, int argc, char **argv);
    DutUnifiedBase(std::initializer_list<const char *> args);
    DutUnifiedBase(std::vector<std::string> args);
    ~DutUnifiedBase();
    void init(int, const char **);
    int simStep();
    int stepNoDump();
    int simStep(bool dump);
    int simStep(uint64_t cycle, bool dump);
    int RefreshComb();
    int Finish();
    uint64_t GetDPIHandle(char *name, int towards);
    uint64_t GetDPIHandle(std::string name, int towards);
    uint64_t GetVPIFuncPtr(const char *name);
    uint64_t GetVPIFuncPtr(std::string name);
    uint64_t GetVPIHandleObj(const char *name);
    uint64_t GetVPIHandleObj(std::string name);
    std::vector<std::string> VPIInternalSignalList(char *name, int depth);
    std::vector<std::string> VPIInternalSignalList(std::string name, int depth);
    void SetWaveform(const char *filename);       // Set waveform file path
    void SetWaveform(const std::string filename); // Set waveform file path
    void FlushWaveform();
    void WaveformEnable(bool enable=true);
    void SetCoverage(const char *filename);       // Set coverage file path
    void SetCoverage(const std::string filename); // Set coverage file path
    int CheckPoint(const char *filename);
    int CheckPoint(const std::string filename);
    int Restore(const char *filename);
    int Restore(const std::string filename);
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
