try:
    from UT_RandomGenerator import *
except:
    try:
        from RandomGenerator import *
    except:
        from __init__ import *

import random

class LSRF_16:
    def __init__(self, seed):
        self.state = seed & ((1 << 16) - 1)

    def step(self):
        new_bit = (self.state >> 15) ^ (self.state >> 14) & 1
        self.state = ((self.state << 1) | new_bit ) & ((1 << 16) - 1)

if __name__ == "__main__":
    dut = DUTRandomGenerator()
    dut.InitClock("clk")

    seed = random.randint(0, 2**16 - 1)

    dut.seed.value = seed
    ref = LSRF_16(seed)
    
    # reset
    dut.reset.value = 1
    dut.Step(1)
    dut.reset.value = 0
    dut.Step(1)

    for i in range(65536):
        print(f"Cycle {i}, DUT: {dut.random_number.value:x}, REF: {ref.state:x}")
        assert dut.random_number.value == ref.state, "Mismatch"
        dut.Step(1)
        ref.step()

    print("Test Passed, destroy UT_RandomGenerator")
    dut.Finish()