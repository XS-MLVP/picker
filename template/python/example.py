from UT_{{__TOP_MODULE_NAME__}} import *

if __name__ == "__main__":
    dut = DUT{{__TOP_MODULE_NAME__}}()
    # dut.init_clock("clk")

    dut.Step(1)

    dut.Finish()