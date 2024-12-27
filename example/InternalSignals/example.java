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
//         print("%2d v1:%3s v2:%3s v3:%3s v4:%3s | _v1:%3s, _v2:%3s, _v3:%3s, _v4:%3s" % (i, dut.v1.value, dut.v2.value, dut.v3.value, dut.v4.value,
//                                                                                         v1.value, v2.value, v3.value, v4.value))
//         input("Press Enter to continue...")
//     dut.Finish()


// rewrite with java

package com.ut;

import com.ut.UT_vpi;
import com.xspcomm.*;

public class example {
    public static void main(String[] args) {
        UT_vpi dut = new UT_vpi(args);
        System.out.println("internal signals: " + dut.VPIInternalSignalList("", 99));
        XData v1 = dut.GetInternalSignal("vpi._v1_base");
        XData v2 = dut.GetInternalSignal("vpi._v2_base");
        XData v3 = dut.GetInternalSignal("vpi._v3_base");
        XData v4 = dut.GetInternalSignal("vpi._v4_base");
        dut.InitClock("clk");
        System.out.println("data size: v1" + v1.W() + " v2:" + v2.W() + " v3:" + v3.W() + " v4:" + v4.W());
        System.out.println("------------------step-------------------");

        for (int i = 0; i < 20; i++) {
            dut.Step(1);
            dut.FlushWaveform();
            if (i == 10) {
                // write to internal signals
                v1.Set(1); // 1 bit, type logic, .W() is 0
                v2.Set(10); // 8 bits
                v3.Set(20); // 32 bits
                v4.Set(30); // 64 bits
            }
            System.out.println(i + " v1:" + dut.v1.U() + " v2:" + dut.v2.U() + " v3:" + dut.v3.U() + " v4:" + dut.v4.U() + " | _v1:" + v1.U() + ", _v2:" + v2.U() + ", _v3:" + v3.U() + ", _v4:" + v4.U());
        }
        dut.Finish();
    }
}