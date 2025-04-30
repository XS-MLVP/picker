#include <bits/stdc++.h>
#include "UT_vpi.hpp"

int main()
{
    UTvpi *dut = new UTvpi("+verilator+debug");
    printf("internal signals: ");
    for (auto &sig : dut->VPIInternalSignalList()) { printf("%s ", sig.c_str()); }
    printf("\n");
    XData *v1 = dut->GetInternalSignal("vpi._v1_base", -1, true);
    XData *v2 = dut->GetInternalSignal("vpi._v2_base", -1, true);
    XData *v3 = dut->GetInternalSignal("vpi._v3_base", -1, true);
    XData *v4 = dut->GetInternalSignal("vpi._v4_base", -1, true);

    dut->InitClock("clk");
    printf("data size: v1:%2d v2:%2d  v3:%2d  v4:%2d\n", v1->W(), v2->W(), v3->W(), v4->W());
    printf("------------------step-------------------\n");

    for (int i = 0; i < 20; i++) {
        dut->Step(1);
        if (i == 10) {
            // write to internal signals
            *v1 = 1;  // 1 bit, type logic, .W() is 0
            *v2 = 10; // 8 bits
            *v3 = 20; // 32 bits
            *v4 = 30; // 64 bits
        }
        printf("%2d v1:%4ld v2:%4ld v3:%4ld v4:%4ld | _v1:%4ld, _v2:%4ld, _v3:%4ld, _v4:%4ld\n",
            i, dut->v1.U(), dut->v2.U(), dut->v3.U(), dut->v4.U(), 
            v1->U(), v2->U(), v3->U(), v4->U());
    }
    dut->Finish();
    return 0;
}