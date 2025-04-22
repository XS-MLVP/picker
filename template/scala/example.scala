package com.ut.{{__TOP_MODULE_NAME__}};

object example {
	def main(args: Array[String]): Unit = {
	  val dut = new UT_{{__TOP_MODULE_NAME__}}();
	  dut.Step(10);
	  dut.Finish();
	  System.out.println("Step {{__TOP_MODULE_NAME__}} 10 times complete");
	}
}
