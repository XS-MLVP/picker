package com.ut;

public class simple_step {
	public static void main(String[] args) {
	  UT_{{__TOP_MODULE_NAME__}} dut = new UT_{{__TOP_MODULE_NAME__}}();
	  for(int i = 0; i<10; i++){
		dut.step();
	  }
	  dut.xFinalize();
	  System.out.println("Step {{__TOP_MODULE_NAME__}} 10 times complete");
	}
}
