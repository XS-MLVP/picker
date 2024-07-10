package com.ut;

import java.math.BigInteger;

// import the generated UT class
import com.ut.UT_Adder;

public class example {
    static public void main(String[] args){
        UT_Adder adder = new UT_Adder();
        for(int i=0; i<10000; i++){
            int N = 1000000;
            long a = (long) (Math.random() * N);
            long b = (long) (Math.random() * N);
            long c = (long) (Math.random() * N) > 50 ? 1 : 0;
            // set inputs
            adder.a.Set(a);
            adder.b.Set(b);
            adder.cin.Set(c);
            // step
            adder.Step();
            // reference model
            long sum = a + b;
            boolean carry = sum < a ? true : false;
            sum += c;
            carry = carry || sum < c;
            // assert
            assert adder.sum.U().longValue() == sum : "sum mismatch: " + adder.sum.U() + " != " + sum;
            assert adder.cout.U().intValue() == (carry ? 1 : 0) : "carry mismatch: " + adder.cout.U() + " != " + carry;
        }
        System.out.println("Java tests passed");
        adder.Finish();
    }
}
