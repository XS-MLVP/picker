package main

import (
	ut "UT_vpi"
	"fmt"
)

func main() {
	dut := ut.NewUT_vpi("+verilator+debug")
	fmt.Println("internal signals:")
	list := dut.VPIInternalSignalList("", 99) // StringVector
	for i := list.Front(); i != nil; i = i.Next() {
		fmt.Println(i.Value)
	}

	v1 := dut.GetInternalSignal("vpi._v1_base", -1, true)
	v2 := dut.GetInternalSignal("vpi._v2_base", -1, true)
	v3 := dut.GetInternalSignal("vpi._v3_base", -1, true)
	v4 := dut.GetInternalSignal("vpi._v4_base", -1, true)
	dut.InitClock("clk")
	fmt.Printf("data size: v1:%2d v2:%2d  v3:%2d  v4:%2d\n", v1.W(), v2.W(), v3.W(), v4.W())
	fmt.Println("------------------step-------------------")

	for i := 0; i < 20; i++ {
		dut.Step(1)
		if i == 10 {
			// write to internal signals
			v1.Set(1)  // 1 bit, type logic, .W() is 0
			v2.Set(10) // 8 bits
			v3.Set(20) // 32 bits
			v4.Set(30) // 64 bits
		}
		fmt.Printf("%2d v1:%3d v2:%3d v3:%3d v4:%3d | _v1:%3d, _v2:%3d, _v3:%3d, _v4:%3d\n", i, dut.V1.U64(), dut.V2.U64(), dut.V3.U64(), dut.V4.U64(),
			v1.U64(), v2.U64(), v3.U64(), v4.U64())
	}
	dut.Finish()
}
