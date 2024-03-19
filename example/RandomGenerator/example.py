from UT_RandomGenerator import *

if __name__ == "__main__":
    dut = DUTRandomGenerator()
    dut.init_clock("clk")

    dut.seed.value = "0x1"

    for i in range(100):
        dut.Step(1)
        print(dut.random_number.value)

    dut.finalize()