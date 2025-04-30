package com.ut;

import com.ut.CacheCFG.UT_CacheCFG;
import com.xspcomm.*;

public class example {
	public static void main(String[] args) {
	  UT_CacheCFG dut = new UT_CacheCFG();
	  dut.GetInternalSignalList().forEach( a -> {
		System.out.println("pin: " + a);
	  });
	  for(int i = 0; i<10; i++){
		dut.Step();
	  }
	  XData valid = dut.GetInternalSignal("CacheCFG_top.io_in_req_valid");
	  System.err.println("valid: " + valid.U().intValue());
	  dut.Finish();
	  System.out.println("Step UT_CacheCFG 10 times complete");
	}
}
