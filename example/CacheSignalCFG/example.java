package com.ut;

public class example {
	public static void main(String[] args) {
	  UT_CacheCFG dut = new UT_CacheCFG();
	  dut.xcfg.GetSignalNames("").forEach( a -> {
		System.out.println("pin: " + a);
	  });
	  for(int i = 0; i<10; i++){
		dut.Step();
	  }
	  dut.Finish();
	  System.out.println("Step UT_CacheCFG 10 times complete");
	}
}
