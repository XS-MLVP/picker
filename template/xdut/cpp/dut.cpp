#include "dut.hpp"

#if defined(USE_VERILATOR)
UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(int argc, char **argv) : DutVerilatorBase(argc, argv)
#elif defined(USE_VCS)
UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(int argc, char **argv) : DutVcsBase(argc, argv)
#endif
{
{{__LOGIC_ANNOTATION__}}

    // initialize {{__TOP_MODULE_NAME__}}
{{__REINIT_PINS__}}

    // bind {{__TOP_MODULE_NAME__}} pins
{{__BIND_PINS__}}

    // add {{__TOP_MODULE_NAME__}} ports
{{__ADD_PORTS__}}
}

#if defined(USE_VERILATOR)
UT{{__TOP_MODULE_NAME__}}::~UT{{__TOP_MODULE_NAME__}}()
#elif defined(USE_VCS)
UT{{__TOP_MODULE_NAME__}}::~UT{{__TOP_MODULE_NAME__}}()
#endif
{
    // finalize {{__TOP_MODULE_NAME__}}
}