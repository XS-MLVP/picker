#pragma once
#include "dut_base.hpp"

#if defined(USE_VERILATOR)
#include "V{{__TOP_MODULE_NAME__}}__Dpi.h"
class DutUnifiedBase : public DutVerilatorBase
#elif defined(USE_VCS)
#include "vc_hdrs.h"
class DutUnifiedBase : public DutVcsBase
#endif
{
public:
    DutUnifiedBase(int argc, char **argv);
    ~DutUnifiedBase();
};

class UT{{__TOP_MODULE_NAME__}}: public DutUnifiedBase
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
