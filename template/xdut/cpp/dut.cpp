#include "dut.hpp"

#if defined(USE_VERILATOR)
UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(int argc, char **argv) : DutVerilatorBase(argc, argv)
#elif defined(USE_VCS)
UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(int argc, char **argv) : DutVcsBase(argc, argv)
#endif
{
    /*
{{__COMMENTS__}}
    */

    // initialize {{__TOP_MODULE_NAME__}}
{{__XDATA_REINIT__}}

    // bind {{__TOP_MODULE_NAME__}} pins
{{__XDATA_BIND__}}

    // add {{__TOP_MODULE_NAME__}} ports
{{__XPORT_ADD__}}
}

#if defined(USE_VERILATOR)
UT{{__TOP_MODULE_NAME__}}::~UT{{__TOP_MODULE_NAME__}}()
#elif defined(USE_VCS)
UT{{__TOP_MODULE_NAME__}}::~UT{{__TOP_MODULE_NAME__}}()
#endif
{
    // finalize {{__TOP_MODULE_NAME__}}
}