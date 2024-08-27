#pragma once
#include "dut_base.hpp"
#include "xspcomm/xcomm.h"
using namespace xspcomm;

class UT{{__TOP_MODULE_NAME__}}
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

    /*******************************/
    /**      User APIs             */
    /*******************************/
    void InitClock(std::string name){this->InitClock(this->xport[name]);}
    void Step(int i = 1){this->xclock.Step(i);}
    void StepRis(std::function<void(uint64_t, void*)> fc, void*args=nullptr){this->xclock.StepRis(fc, args);}
    void StepFal(std::function<void(uint64_t, void*)> fc, void*args=nullptr){this->xclock.StepFal(fc, args);}
    void SetWaveform(std::string filename){this->dut->SetWaveform((const char*)filename.c_str());}
    void SetCoverage(std::string filename){this->dut->SetCoverage((const char*)filename.c_str());}
    void Finish(){this->dut->Finish();}
    void RefreshComb(){this->dut->RefreshComb();}
    /*******************************/
    /**    End of User APIs        */
    /*******************************/

};
