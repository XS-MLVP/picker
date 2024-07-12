#include "UT_{{__TOP_MODULE_NAME__}}.hpp"

int main()
{
#if defined(USE_VCS)
    UT{{__TOP_MODULE_NAME__}} *dut = new UT{{__TOP_MODULE_NAME__}}("libDPI{{__TOP_MODULE_NAME__}}.so");
#elif defined(USE_VERILATOR)
    UT{{__TOP_MODULE_NAME__}} *dut = new UT{{__TOP_MODULE_NAME__}}();
#endif
    dut->xclock.Step(1);
    printf("Initialized UT{{__TOP_MODULE_NAME__}}\n");
    delete dut;
    printf("Deleted UT{{__TOP_MODULE_NAME__}}\n");
    return 0;
}