package com.ut

import com.ut.UT_RandomGenerator
import Math.random


object example {
  def main(args: Array[String]): Unit = {
    val dut = new UT_RandomGenerator()
    val seed = (random() * 100000).toInt
    var state = seed & ((1 << 16) - 1)
    // init clock
    dut.InitClock("clk")
    // set random seed
    dut.seed.Set(seed)
    // reset
    dut.reset.Set(1)
    dut.Step(1)
    dut.reset.Set(0)
    dut.Step(1)
    for (i <- 0 until 10) {
      // assert
      assert(dut.random_number.U().intValue() == state, "rand mismatch")
      dut.Step()
      // reference model update
      val new_bit = (state >> 15) ^ (state >> 14) & 1
      state = ((state << 1) | new_bit) & ((1 << 16) - 1)
    }
    println("10 random scala tests passed")
    dut.Finish()
  }
}
