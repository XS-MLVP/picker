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
{{__LOGIC_ANNOTATION__}}

    // {{__TOP_MODULE_NAME__}} INPUT
{{__INPUT_PINS__}}

    // {{__TOP_MODULE_NAME__}} OUTPUT
{{__OUTPUT_PINS__}}

    XPort port;

    // {{__TOP_MODULE_NAME__}}
    UT{{__TOP_MODULE_NAME__}}(int argc, char **argv);
    ~UT{{__TOP_MODULE_NAME__}}();
};
