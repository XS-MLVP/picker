package main

import (
	"fmt"
    "time"
    "math/rand"
    ut "UT_Adder"
)

func assert(cond bool, msg string) {
    if !cond {
        panic(msg)
    }
}

func main() {
    adder := ut.NewUT_Adder()
    rand.Seed(time.Now().UnixNano())
    for i := 0; i < 10000; i++ {
        N := 1000000
        a := rand.Int63n(int64(N))
        b := rand.Int63n(int64(N))
        var c int64
        if rand.Int63n(int64(N)) > 50 {
            c = 1
        } else {
            c = 0
        }

        adder.A.Set(a)
        adder.B.Set(b)
        adder.Cin.Set(c)

        adder.Step()

        // reflerence model
        sum := a + b
        carry := sum < a
        sum += c
        carry = carry || sum < c

        // assert
        assert(adder.Sum.U64() == uint64(sum), fmt.Sprintf("sum mismatch: %d != %d\n", adder.Sum.U64(), uint64(sum)))
        
        var carry_bool uint64
        if carry {
            carry_bool = 1
        } else {
            carry_bool = 0
        }
        assert(adder.Cout.U64() == carry_bool, fmt.Sprintf("carry mismatch: %d != %t\n", adder.Cout.U().Int64(), carry))
    }
    adder.Finish();
    fmt.Println("Golang tests passed")
}
