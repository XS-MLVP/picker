#include <mvwave.h>
#include <verilated.h>
#include <verilated_vcd_c.h>


//=============================================================================
// MVWaveFile
/// Class representing a file to write to. 

MVWaveFile::MVWaveFile()
{
    this->wavefilep_ = new VerilatedVcdFile();
}

MVWaveFile::~MVWaveFile()
{
    delete (VerilatedVcdFile*)(this->wavefilep_);
}

bool MVWaveFile::open(const char* name)
{
    return ((VerilatedVcdFile*)(this->wavefilep_))->open(name);
}

void MVWaveFile::close()
{
    ((VerilatedVcdFile*)(this->wavefilep_))->close();
}

ssize_t MVWaveFile::write(const char *bufp, ssize_t len)
{
    return ((VerilatedVcdFile*)(this->wavefilep_))->write(bufp, len);
}

//=============================================================================
// MVWaveC
/// Class representing a VCD dump file in C standalone (no SystemC)
/// simulations.  Also derived for use in SystemC simulations.

MVWaveC::MVWaveC(MVWaveFile *filep)
{
    this->tracep_ = new VerilatedVcdC((VerilatedVcdFile *)(filep->wavefilep_));
}

MVWaveC::~MVWaveC()
{
    delete (VerilatedVcdC*)(this->tracep_);
}

bool MVWaveC::isOpen() const
{
    return ((VerilatedVcdC*)(this->tracep_))->isOpen();
}

void MVWaveC::open(const char *filename)
{
    ((VerilatedVcdC*)(this->tracep_))->open(filename);
}

void MVWaveC::openNext(bool incFilename)
{
    ((VerilatedVcdC*)(this->tracep_))->openNext(incFilename);
}

void MVWaveC::rolloverSize(size_t size)
{
    ((VerilatedVcdC*)(this->tracep_))->rolloverSize(size);
}

void MVWaveC::close()
{
    ((VerilatedVcdC*)(this->tracep_))->close();
}

void MVWaveC::flush()
{
    ((VerilatedVcdC*)(this->tracep_))->flush();
}

void MVWaveC::dump(uint64_t timeui)
{
    ((VerilatedVcdC*)(this->tracep_))->dump(timeui);
}

void MVWaveC::dump(double timestamp)
{
    ((VerilatedVcdC*)(this->tracep_))->dump(timestamp);
}

void MVWaveC::dump(uint32_t timestamp)
{
    ((VerilatedVcdC*)(this->tracep_))->dump(timestamp);
}

void MVWaveC::dump(int timestamp)
{
    ((VerilatedVcdC*)(this->tracep_))->dump(timestamp);
}

void MVWaveC::set_time_unit(const char *unit)
{
    ((VerilatedVcdC*)(this->tracep_))->set_time_unit(unit);
}

void MVWaveC::set_time_resolution(const char *unit)
{
    ((VerilatedVcdC*)(this->tracep_))->set_time_resolution(unit);
}

void MVWaveC::dumpvars(int level, const char *hier)
{
    ((VerilatedVcdC*)(this->tracep_))->dumpvars(level, hier);
}