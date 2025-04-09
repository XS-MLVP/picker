package com.ut;

object example {
	def main(args: Array[String]): Unit = {
	  val dut = new UT_CacheCFG();
	  for(p <- dut.GetInternalSignalList()){
        println("pin: " + p)
      }
      val clk = dut.GetInternalSignal("CacheCFG_top.clock")
      println("clk: " + clk.AsInt32())
	  dut.Finish();
	  System.out.println("Step UT_CacheCFG 10 times complete");
	}
}
