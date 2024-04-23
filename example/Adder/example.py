from UT_Adder import *

if __name__ == "__main__":
    dut = DUTAdder()
    # dut.init_clock("clk")

    for i in range(6):
        dut.a.value = i * 2
        dut.b.value = int(i / 4)
        dut.Step(1)
        print(dut.sum.value, dut.cout.value)

    dut.finalize()