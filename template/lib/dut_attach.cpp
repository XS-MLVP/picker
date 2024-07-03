#include "dut_attach.hpp"

V{{__TOP_MODULE_NAME__}} * dlcreates(VerilatedContext *vc)
{
    return new V{{__TOP_MODULE_NAME__}} {vc};
}
