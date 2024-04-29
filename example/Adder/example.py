from UT_Adder import *

import random

class input_t:
    def __init__(self, a, b, cin):
        self.a = a
        self.b = b
        self.cin = cin

class output_t:
    def __init__(self):
        self.sum = 0
        self.cout = 0

def random_int():
    return random.randint(-(2**127), 2**127 - 1) & ((1 << 127) - 1)

def as_uint(x, nbits):
    return x & ((1 << nbits) - 1)

def main():
    dut = DUTAdder()  # Assuming USE_VERILATOR
    
    print("Initialized UTAdder")
    
    for c in range(114514):
        i = input_t(random_int(), random_int(), random_int() & 1)
        o_dut, o_ref = output_t(), output_t()
        
        def dut_cal():
            dut.a.value, dut.b.value, dut.cin.value = i.a, i.b, i.cin
            dut.Step(1)
            o_dut.sum = dut.sum.value
            o_dut.cout = dut.cout.value
        
        def ref_cal():
            sum = as_uint( i.a + i.b, 256 )
            carry = sum < i.a
            sum += i.cin
            carry = carry or sum < i.cin
            o_ref.sum, o_ref.cout = sum, carry
        
        dut_cal()
        ref_cal()
        
        print(f"[cycle {dut.xclock.clk}] a=0x{i.a:x}, b=0x{i.b:x}, cin=0x{i.cin:x}")
        print(f"DUT: sum=0x{o_dut.sum:x}, cout=0x{o_dut.cout:x}")
        print(f"REF: sum=0x{o_ref.sum:x}, cout=0x{o_ref.cout:x}")
        
        assert o_dut.sum == o_ref.sum, "sum mismatch"

    dut.finalize()
    
    print("Test Passed, destroy UTAdder")

if __name__ == "__main__":
    main()