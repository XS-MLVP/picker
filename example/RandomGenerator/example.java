package com.ut;

import com.ut.UT_RandomGenerator;

public class example {
        static public void main(String[] args){
        UT_RandomGenerator dut = new UT_RandomGenerator();
        int seed = (int) (Math.random() * 100000);
        int state = seed & ((1 << 16) - 1);        
        // init clocl
        dut.InitClock("clk");
        // set random seed
        dut.seed.Set(seed);
        // reset    
        dut.reset.Set(1);
        dut.Step(1);
        dut.reset.Set(0);
        dut.Step(1);
        for(int i=0; i<10; i++){
            // assert
            assert dut.random_number.U().intValue() == state : "rand mismatch";
            dut.Step();
            // reference model update
            int new_bit = (state >> 15) ^ (state >> 14) & 1;
            state = ((state << 1) | new_bit ) & ((1 << 16) - 1);
        }
        System.out.println("All tests passed");
        dut.Finish();
    }
}
