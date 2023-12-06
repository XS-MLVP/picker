#pragma once
#include "dut_base.hpp"
#include "xspcomm/xcomm.h"
using namespace xspcomm;

class UT{{__TOP_MODULE_NAME__}}: public DutUnifiedBase
{
public:
    /*
{{__COMMENTS__}}
    */

    // {{__TOP_MODULE_NAME__}} 
{{__XDATA_DECLARATION__}}

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
    void init();
    void initClock(XData &clk);
};
