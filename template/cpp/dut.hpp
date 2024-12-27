#pragma once
#include "dut_base.hpp"
#include "xspcomm/xcomm.h"
using namespace xspcomm;
// clang-format off
class UT{{__TOP_MODULE_NAME__}}
{
    void init();
public:
    /*
{{__COMMENTS__}}
    */

    // {{__TOP_MODULE_NAME__}} 
{{__XDATA_DECLARATION__}}

    // signal hashmap
    std::map<std::string, XData*> signal_map;

    // DUT
    DutUnifiedBase *dut = nullptr;

    // Ports for binding
    XPort xport;

    // Sub ports
{{__XPORT_CASCADED_DEC__}}

    // Clock for DUT timing
    XClock xclock;

    // {{__TOP_MODULE_NAME__}}
    UT{{__TOP_MODULE_NAME__}}();
    UT{{__TOP_MODULE_NAME__}}(int argc, char **argv);
    UT{{__TOP_MODULE_NAME__}}(char *filename);
    UT{{__TOP_MODULE_NAME__}}(char *filename, int argc, char **argv);
    UT{{__TOP_MODULE_NAME__}}(std::initializer_list<const char *> args);
    ~UT{{__TOP_MODULE_NAME__}}();

    // Funtions for DUT
    void InitClock(XData &clk);
    // clang-format on

    /*******************************/
    /**      User APIs             */
    /*******************************/
    void InitClock(std::string name)
    {
        this->InitClock(this->xport[name]);
    }
    void Step(int i = 1)
    {
        this->xclock.Step(i);
    }
    void StepRis(std::function<void(uint64_t, void *)> fc, void *args = nullptr)
    {
        this->xclock.StepRis(fc, args);
    }
    void StepFal(std::function<void(uint64_t, void *)> fc, void *args = nullptr)
    {
        this->xclock.StepFal(fc, args);
    }
    void SetWaveform(std::string filename)
    {
        this->dut->SetWaveform((const char *)filename.c_str());
    }
    void FlushWaveform()
    {
        this->dut->FlushWaveform();
    }
    void SetCoverage(std::string filename)
    {
        this->dut->SetCoverage((const char *)filename.c_str());
    }
    void CheckPoint(std::string filename)
    {
        this->dut->CheckPoint((const char *)filename.c_str());
    }
    void Restore(std::string filename)
    {
        this->dut->Restore((const char *)filename.c_str());
    }
    XData *GetInternalSignal(std::string name)
    {
        // if not found, create a new one
        if (this->signal_map.find(name) == this->signal_map.end()) {
            this->signal_map[name] = XData::FromVPI(this->dut->GetVPIHandleObj(name), this->dut->GetVPIFuncPtr("vpi_get"),
                                     this->dut->GetVPIFuncPtr("vpi_get_value"),
                                     this->dut->GetVPIFuncPtr("vpi_put_value"), name);
        }
        return this->signal_map[name];
    }
    std::vector<std::string> VPIInternalSignalList(std::string name = "", int depth = 99)
    {
        return this->dut->VPIInternalSignalList(name, depth);
    }
    void Finish()
    {
        this->dut->Finish();
    }
    void RefreshComb()
    {
        this->dut->RefreshComb();
    }
    /*******************************/
    /**    End of User APIs        */
    /*******************************/
};
