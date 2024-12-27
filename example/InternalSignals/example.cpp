// try:
//     from UT_vpi import *
// except:
//     try:
//         from vpi import *
//     except:
//         from __init__ import *

// if __name__ == "__main__":
//     dut = DUTvpi("+verilator+debug")
//     print("internal signals: ", dut.VPIInternalSignalList())
//     v1 = dut.GetInternalSignal("vpi._v1_base")
//     v2 = dut.GetInternalSignal("vpi._v2_base")
//     v3 = dut.GetInternalSignal("vpi._v3_base")
//     v4 = dut.GetInternalSignal("vpi._v4_base")
//     dut.InitClock("clk")
//     print("data size: v1:%2d v2:%2d  v3:%2d  v4:%2d" % (v1.W(), v2.W(), v3.W(), v4.W()))
//     print("------------------step-------------------")

//     for i in range(20):
//         dut.Step(1)
//         dut.FlushWaveform()
//         if i == 10:
//             # write to internal signals
//             v1.value = 1          # 1 bit, type logic, .W() is 0
//             v2.value = 10         # 8 bits
//             v3.value = 20         # 32 bits
//             v4.value = 30         # 64 bits
//         print("%2d v1:%3s v2:%3s v3:%3s v4:%3s | _v1:%3s, _v2:%3s, _v3:%3s, _v4:%3s" % (i, dut.v1.value,
//         dut.v2.value, dut.v3.value, dut.v4.value,
//                                                                                         v1.value, v2.value, v3.value,
//                                                                                         v4.value))
//         input("Press Enter to continue...")
//     dut.Finish()

#include <bits/stdc++.h>
#include "UT_vpi.hpp"

int main()
{
    UTvpi *dut = new UTvpi("+verilator+debug");
    printf("internal signals: ");
    for (auto &sig : dut->VPIInternalSignalList()) { printf("%s ", sig.c_str()); }
    printf("\n");
    XData *v1 = dut->GetInternalSignal("vpi._v1_base");
    XData *v2 = dut->GetInternalSignal("vpi._v2_base");
    XData *v3 = dut->GetInternalSignal("vpi._v3_base");
    XData *v4 = dut->GetInternalSignal("vpi._v4_base");

    dut->InitClock("clk");
    printf("data size: v1:%2d v2:%2d  v3:%2d  v4:%2d\n", v1->W(), v2->W(), v3->W(), v4->W());
    printf("------------------step-------------------\n");

    for (int i = 0; i < 20; i++) {
        dut->Step(1);
        dut->FlushWaveform();
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