package com.ut

import java.lang.Math
import com.ut.Adder.UT_Adder

object example {
    def main(args: Array[String]): Unit = {
      val adder = new UT_Adder()
      for (i <- 0 until 10000) {
        val N = 1000000
        val a = (Math.random() * N).toLong
        val b = (Math.random() * N).toLong
        val c = if ((Math.random() * N).toLong > 50) 1 else 0

        // set inputs
        adder.a.Set(a)
        adder.b.Set(b)
        adder.cin.Set(c)

        // step
        adder.Step()

        // reference model
        var sum = a + b
        var carry = if (sum < a) true else false
        sum += c
        carry = carry || (sum < c)

        // assert
        assert(adder.sum.U().longValue() == sum, s"sum mismatch: ${adder.sum.U()} != $sum")
        assert(adder.cout.U().intValue() == (if (carry) 1 else 0), s"carry mismatch: ${adder.cout.U()} != $carry")
        println(s"[cycle ${adder.xclock.getClk().intValue()}] a=${adder.a.U64()}, b=${adder.b.U64()}, cin=${adder.cin.U64()}")
      }
      println("Scala tests passed")
      adder.Finish()
    }
}
