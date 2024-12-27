package com.ut
import com.ut.UT_vpi

object example {
  def main(args: Array[String]): Unit = {
    val dut = new UT_vpi(args)
    println("internal signals: " + dut.VPIInternalSignalList("", 99))
    val v1 = dut.GetInternalSignal("vpi._v1_base")
    val v2 = dut.GetInternalSignal("vpi._v2_base")
    val v3 = dut.GetInternalSignal("vpi._v3_base")
    val v4 = dut.GetInternalSignal("vpi._v4_base")
    dut.InitClock("clk")
    println(s"data size: v1${v1.W()} v2:${v2.W()} v3:${v3.W()} v4:${v4.W()}")
    println("------------------step-------------------")

    for (i <- 0 until 20) {
      dut.Step(1)
      dut.FlushWaveform()
      if (i == 10) {
        // write to internal signals
        v1.Set(1) // 1 bit, type logic, .W() is 0
        v2.Set(10) // 8 bits
        v3.Set(20) // 32 bits
        v4.Set(30) // 64 bits
      }
      println(s"$i v1:${dut.v1.U()} v2:${dut.v2.U()} v3:${dut.v3.U()} v4:${dut.v4.U()} | _v1:${v1.U()}, _v2:${v2.U()}, _v3:${v3.U()}, _v4:${v4.U()}")
    }
    dut.Finish()
  }
}
