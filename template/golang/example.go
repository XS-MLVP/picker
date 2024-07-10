package main

import (
	"fmt"
    ut "UT_{{__TOP_MODULE_NAME__}}"
)

func main() {
    dut := ut.NewUT_{{__TOP_MODULE_NAME__}}()
	dut.Step(100)
    dut.Finish()
    fmt.Println("Step {{__TOP_MODULE_NAME__}} 100 times.")
}
