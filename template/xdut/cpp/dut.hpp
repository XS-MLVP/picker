#pragma once
#include "dut_base.hpp"

#if defined(USE_VERILATOR)
#include "V{{__TOP_MODULE_NAME__}}__Dpi.h"
class UT{{__TOP_MODULE_NAME__}} : public DutVerilatorBase
#elif defined(USE_VCS)
#include "vc_hdrs.h"
class UT{{__TOP_MODULE_NAME__}} : public DutVcsBase
#endif
{
public:
    /*
{{__COMMENTS__}}
    */

    // {{__TOP_MODULE_NAME__}} 
{{__XDATA_DECLARATION__}}


    XPort port;

    // {{__TOP_MODULE_NAME__}}
    UT{{__TOP_MODULE_NAME__}}(int argc, char **argv);
    ~UT{{__TOP_MODULE_NAME__}}();
};
