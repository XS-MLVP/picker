#include "UT_{{__TOP_MODULE_NAME__}}.hpp"

int main()
{
    UT{{__TOP_MODULE_NAME__}} *dut = new UT{{__TOP_MODULE_NAME__}}();
    dut->xclock.Step(1);
    printf("Initialized UT{{__TOP_MODULE_NAME__}}\n");
    delete dut;
    printf("Deleted UT{{__TOP_MODULE_NAME__}}\n");
    return 0;
}