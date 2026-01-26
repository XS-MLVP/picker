#pragma once
#include <cstdio>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <initializer_list>

#if defined(USE_VERILATOR) || defined(USE_VCS)
#include <svdpi.h>
#endif

#if defined(USE_GSIM)
#include "{{__TOP_MODULE_NAME__}}.h"
#include "dut_type.hpp"
#endif

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
    virtual void FlushWaveform()                   = 0;
    virtual bool ResumeWaveformDump()              = 0;
    virtual bool PauseWaveformDump()               = 0;
    virtual void WaveformEnable(bool enable)       = 0;
    // Set coverage file path
    virtual void SetCoverage(const char *filename) = 0;
    // Save Model Status with Simulator Capabilities
    virtual int CheckPoint(const char *filename) = 0;
    // Load Model Status with Simulator Capabilities
    virtual int Restore(const char *filename) = 0;
    // Fast get Pin Mem address and size
    virtual uint64_t NativeSignalAddr(const char *name) = 0;
};

#if defined(USE_GSIM)
class DutGSimBase : public DutBase
{
public:
    S{{__TOP_MODULE_NAME__}} *top = nullptr; // GSim Top Module
private:
    std::string coverage_file_path;
    // shadow pin values
    {% if __SIMULATOR__ == "gsim" %}
    {% for pin in __MODULE_EXTERNAL_PINS__ %}{% if pin.type == "Out" %}
    type_{{pin.name}} vpin_{{pin.name}} = 0;
    {% else %}
    type_{{pin.name}} vpin_{{pin.name}} = 0;
    {% endif %}{% endfor %}
    {% endif %}
public:
    std::map<std::string, uint64_t> pin_address_map;
    std::map<std::string, int> pin_size_map;
    void init(int, char **);
    DutGSimBase();
    DutGSimBase(int argc, char **argv);
    ~DutGSimBase();
    int Step(uint64_t cycle, bool dump);
    int Finish();
    void SetWaveform(const char *filename);
    void FlushWaveform();
    bool ResumeWaveformDump();
    bool PauseWaveformDump();
    void WaveformEnable(bool enable);
    void SetCoverage(const char *filename);
    int CheckPoint(const char *filename);
    int Restore(const char *filename);
    uint64_t NativeSignalAddr(const char *name);
    void update_read();
    void update_write();
};
extern "C" {
DutGSimBase *dlcreates(int argc, char **argv);
void dlstep(DutGSimBase *dut, uint64_t ncycle, bool dump);
}
#endif

#if defined(USE_VERILATOR)
class DutVerilatorBase : public DutBase
{
private:
    std::string coverage_file_path;
    bool wave_pause_deferred = false;
    bool wave_pause_warned   = false;

public:
    std::map<std::string, uint64_t> pin_address_map;
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
    bool ResumeWaveformDump();
    bool PauseWaveformDump();
    void WaveformEnable(bool enable);
    void SetCoverage(const char *filename);
    int CheckPoint(const char *filename);
    int Restore(const char *filename);
    uint64_t NativeSignalAddr(const char *name);
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
    bool ResumeWaveformDump();
    bool PauseWaveformDump();
    void WaveformEnable(bool enable);
    void SetCoverage(const char *filename);
    int CheckPoint(const char *filename);
    int Restore(const char *filename);
    uint64_t NativeSignalAddr(const char *name);
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
    
    // Bitmask of the enabled code coverage metrics, configured at compile time. 0 indicates coverage is disabled.
    static const int coverage_metrics;

    // File extension for the waveform trace file (e.g., "vcd", "fsdb"), configured at compile time.
    // This will be an empty string if waveform dumping is disabled.
    static const char waveform_format[];

    // Flag to track if waveform dumping is paused. 0 means active, 1 means paused/closed.
    int waveform_paused;

#if defined(USE_VERILATOR)
    DutVerilatorBase *dut;
#elif defined(USE_VCS)
    DutVcsBase *dut;
#elif defined(USE_GSIM)
    DutGSimBase *dut;
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

    // Get Library Path for Locate XSignalCFG ymal config
    std::string GetXSignalCFGPath();
    uint64_t GetXSignalCFGBasePtr();

    // Set waveform file path
    void SetWaveform(const char *filename);
    void SetWaveform(const std::string filename);

    // Get the waveform extension, or an empty string if disabled.
    static std::string GetWaveFormat();
    
    // Flush waveform from mem cache to file
    void FlushWaveform();

    // Set whether waveform is logged to file or not (time still pass)
    void WaveformEnable(bool enable = true);

    // Open and close waveform file
    bool ResumeWaveformDump();
    bool PauseWaveformDump();

    // Returns 1 if waveform export is paused
    int WaveformPaused();

    // Set coverage file path
    void SetCoverage(const char *filename);
    void SetCoverage(const std::string filename);

    // Get the bitmask for collected coverage metrics. 0 means coverage is disabled
    static int GetCovMetrics();

    // Save Model Status with Simulator Capabilities
    // Return current cycle, warning: strictly limited to same design
    int CheckPoint(const char *filename);
    int CheckPoint(const std::string filename);

    // Load Model Status with Simulator Capabilities
    // Return current cycle, warning: strictly limited to same design
    int Restore(const char *filename);
    int Restore(const std::string filename);

    // Fast get Pin Mem address and size for mem direct access mode
    uint64_t NativeSignalAddr(const char *name);
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
