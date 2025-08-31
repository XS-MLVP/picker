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
    std::map<std::string, std::vector<std::shared_ptr<XData>>> signal_list_map;

    // DUT
    DutUnifiedBase *dut = nullptr;

    // Ports for binding
    XPort xport;

    // Sub ports
{{__XPORT_CASCADED_DEC__}}

    // Clock for DUT timing
    XClock xclock;

    XSignalCFG *xcfg = nullptr;

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
    XClock* GetXClock()
    {
        return &this->xclock;
    }
    XPort* GetXPort()
    {
        return &this->xport;
    }
    bool ResumeWaveformDump()
    {
        return this->dut->ResumeWaveformDump();
    }
    bool PauseWaveformDump()
    {
        return this->dut->PauseWaveformDump();
    }
    int WaveformPaused()
    {
        return this->dut->WaveformPaused();
    }
    void SetWaveform(std::string filename)
    {
        this->dut->SetWaveform((const char *)filename.c_str());
    }
    std::string GetWaveFormat()
    {
        return this->dut->GetWaveFormat();
    } 
    void FlushWaveform()
    {
        this->dut->FlushWaveform();
    }
    void SetCoverage(std::string filename)
    {
        this->dut->SetCoverage((const char *)filename.c_str());
    }
    int GetCovMetrics()
    {
        return this->dut->GetCovMetrics();
    }
    void CheckPoint(std::string filename)
    {
        this->dut->CheckPoint((const char *)filename.c_str());
    }
    void Restore(std::string filename)
    {
        this->dut->Restore((const char *)filename.c_str());
    }
    XData *GetInternalSignal(std::string name, int index = -1, bool use_vpi = false)
    {
        // if not found, create a new one
        if (this->signal_map.find(name) == this->signal_map.end()) {
            XData *signal = nullptr;
            if(!use_vpi){
                std::string xname = "CFG:" + name;
                if (this->dut->GetXSignalCFGBasePtr()==0)return nullptr;
                if(index >= 0){
                    signal = this->xcfg->NewXData(name, index, xname);
                }else{
                    signal = this->xcfg->NewXData(name, xname);
                }
            }else{
                signal = XData::FromVPI(this->dut->GetVPIHandleObj(name), this->dut->GetVPIFuncPtr("vpi_get"),
                                     this->dut->GetVPIFuncPtr("vpi_get_value"),
                                     this->dut->GetVPIFuncPtr("vpi_put_value"), "VPI:"+name);
            }
            if(signal == nullptr){
                return nullptr;
            }
            this->signal_map[name] = signal;
        }
        return this->signal_map[name];
    }
    std::vector<std::shared_ptr<XData>> GetInternalSignal(std::string name, bool is_array){
        Assert(is_array==true, "GetInternalSignal: is_array must be true");
        if(this->signal_list_map.find(name) == this->signal_list_map.end()){
            std::vector<std::shared_ptr<XData>> signal = this->xcfg->NewXDataArray(name, "CFG:" + name);
            if(signal.size() == 0){
                return signal;
            }
            this->signal_list_map[name] = signal;
        }
        return this->signal_list_map[name];
    }
    std::vector<std::string> GetInternalSignalList(std::string prefix = "", int deep = 99, bool use_vpi = false){
        if(!use_vpi){
            return this->xcfg->GetSignalNames(prefix);
        }
        return this->dut->VPIInternalSignalList(prefix, deep);
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
