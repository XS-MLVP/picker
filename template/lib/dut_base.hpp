#pragma once
#include <bits/stdc++.h>
#include <svdpi.h>
#include <dlfcn.h>

class DutBase
{
public:
    uint64_t cycle;
    int argc;
    void *lib_handle;
    char **argv;

    // Initialize
    DutBase();
    virtual ~DutBase() = default;

    // Simulate 1 cycle
    virtual int step() = 0;

    // Simulate N cycles
    virtual int step(uint64_t cycle, bool dump) = 0;
    // Clean up and dump result
    virtual int finished() = 0;
    virtual uint64_t get_dpi_handle(char* name, int towards) = 0;

    // Set waveform file path
    virtual void set_waveform(const char *filename) = 0;
    // Set coverage file path
    virtual void set_coverage(const char *filename) = 0;
};

#if defined(USE_VERILATOR)
class DutVerilatorBase : public DutBase
{
private:
    std::string coverage_file_path;
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
    int finished();
    uint64_t get_dpi_handle(char* name, int towards);
    void set_waveform(const char *filename);
    void set_coverage(const char *filename);
};

#endif

#if defined(USE_VCS)
extern "C" {
int VcsMain(int argc, char **argv);
void VcsInit();
void VcsSimUntil(uint64_t *);
}

class DutVcsBase : public DutBase
{
protected:
    uint64_t cycle_hl;
    uint64_t vcs_clock_period[3];

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
    int finished();
    void set_waveform(const char *filename);
    void set_coverage(const char *filename);
};

#endif

#if defined(USE_VERILATOR)
class DutUnifiedBase : public DutVerilatorBase
#elif defined(USE_VCS)
class DutUnifiedBase : public DutVcsBase
#endif
{
public:
    DutUnifiedBase();
    DutUnifiedBase(int argc, char **argv);
    DutUnifiedBase(char *filename);
    DutUnifiedBase(char *filename, int argc, char **argv);
    DutUnifiedBase(std::initializer_list<const char *> args);
    ~DutUnifiedBase();
    int finished();
    uint64_t get_dpi_handle(char* name, int towards);
    void set_waveform(const char *filename); // Set waveform file path
    void set_coverage(const char *filename); // Set coverage file path
};
