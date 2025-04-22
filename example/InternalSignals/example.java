
package com.ut;

import com.ut.vpi.UT_vpi;
import com.xspcomm.*;

public class example {
    public static void main(String[] args) {
        UT_vpi dut = new UT_vpi(args);
        System.out.println("internal signals: " + dut.VPIInternalSignalList("", 99));
        XData v1 = dut.GetInternalSignal("vpi._v1_base", -1, true);
        XData v2 = dut.GetInternalSignal("vpi._v2_base", -1, true);
        XData v3 = dut.GetInternalSignal("vpi._v3_base", -1, true);
        XData v4 = dut.GetInternalSignal("vpi._v4_base", -1, true);
        dut.InitClock("clk");
        System.out.println("data size: v1" + v1.W() + " v2:" + v2.W() + " v3:" + v3.W() + " v4:" + v4.W());
        System.out.println("------------------step-------------------");

        for (int i = 0; i < 20; i++) {
            dut.Step(1);
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
