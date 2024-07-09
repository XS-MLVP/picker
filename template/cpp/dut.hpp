#pragma once
#include "dut_base.hpp"
#include "xspcomm/xcomm.h"
using namespace xspcomm;

class UT{{__TOP_MODULE_NAME__}}: public DutUnifiedBase
{
    void init();
public:
    /*
{{__COMMENTS__}}
    */

    // {{__TOP_MODULE_NAME__}} 
{{__XDATA_DECLARATION__}}

    // DUT
    DutUnifiedBase *dut = nullptr;

    // Ports for binding
    XPort port;

    // Clock for DUT timing
    XClock xclk;

    // {{__TOP_MODULE_NAME__}}
    UT{{__TOP_MODULE_NAME__}}();
    UT{{__TOP_MODULE_NAME__}}(int argc, char **argv);
    UT{{__TOP_MODULE_NAME__}}(char *filename);
    UT{{__TOP_MODULE_NAME__}}(char *filename, int argc, char **argv);
    UT{{__TOP_MODULE_NAME__}}(std::initializer_list<const char *> args);
    ~UT{{__TOP_MODULE_NAME__}}();

    // Funtions for DUT
    void InitClock(XData &clk);

    /*******************************/
    /**      User APIs             */
    /*******************************/
    void InitClock(std::string name){this->InitClock(this->port[name]);}
    void Step(int i = 1){this->dut->simStep(i);}
    void StepRis(std::function<void(uint64_t, void*)> fc, void*args=nullptr){this->xclk.StepRis(fc, args);}
    void StepFal(std::function<void(uint64_t, void*)> fc, void*args=nullptr){this->xclk.StepFal(fc, args);}
    void SetWaveform(std::string filename){this->dut->SetWaveform((const char*)filename.c_str());}
    void SetCoverage(std::string filename){this->dut->SetCoverage((const char*)filename.c_str());}
    void Finish(){this->dut->Finish();}
    void RefreshComb(){this->dut->RefreshComb();}
    /*******************************/
    /**    End of User APIs        */
    /*******************************/

};
