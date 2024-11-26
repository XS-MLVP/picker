package main

import (
	"fmt"
    "time"
    "math/rand"
    ut "UT_RandomGenerator"
)

func assert(cond bool, msg string) {
    if !cond {
        panic(msg)
    }
}

func main() {
  rand.Seed(time.Now().UnixNano())
  dut := ut.NewUT_RandomGenerator()
  seed := rand.Intn(100000)
  state := seed & ((1 << 16) - 1)

  // init clock
  dut.InitClock("clk")

  // set random seed
  dut.Seed.Set(uint64(seed))

  // reset
  dut.Reset.Set(1)
  dut.Step(1)
  dut.Reset.Set(0)
  dut.Step(1)

  for i := 0; i < 10; i++ {
      // assert
      if int(dut.Random_number.Get().Int64()) != state {
          fmt.Printf("rand mismatch: expected %d, got %d\n", state, dut.Random_number.Get().Int64())
      }
      dut.Step(1)

      // reference model update
      newBit := (state >> 15) ^ (state >> 14) & 1
      state = ((state << 1) | newBit) & ((1 << 16) - 1)
  }

  fmt.Println("10 random Go tests passed")
  dut.Finish()
}

