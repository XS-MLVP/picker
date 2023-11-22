#include "dut.hpp"


#if defined(USE_VERILATOR)
DutUnifiedBase::DutUnifiedBase(int argc, char **argv) : DutVerilatorBase(argc, argv)
#elif defined(USE_VCS)
DutUnifiedBase::DutUnifiedBase(int argc, char **argv) : DutVcsBase(argc, argv)
#endif
{};

UT{{__TOP_MODULE_NAME__}}::UT{{__TOP_MODULE_NAME__}}(int argc, char **argv) : DutUnifiedBase(argc, argv)
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

DutUnifiedBase::~DutUnifiedBase()
{
    // finalize {{__TOP_MODULE_NAME__}}
}

UT{{__TOP_MODULE_NAME__}}::~UT{{__TOP_MODULE_NAME__}}()
{
    // finalize {{__TOP_MODULE_NAME__}}
}