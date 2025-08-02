try:
    from UT_SimTop import *
except:
    try:
        from SimTop import *
    except:
        from __init__ import *

from xspcomm import ComUseDataArray

def load_bin(dut: DUTSimTop, bin_file: str):
    try:
        with open(bin_file, 'rb') as f:
            data = f.read()
    except FileNotFoundError:
        print(f"Error: File {bin_file} not found.")
        return False
    address = dut.xcfg.Address("mem$rdata_mem$mem")
    bin_array = ComUseDataArray(address, len(data))
    bin_array.FromBytes(data)
    print(f"Loading {bin_file} ({len(data)} bytes) to address {hex(address)}")
    return True

def init(dut: DUTSimTop):
    dut.difftest_logCtrl_begin.value = 0
    dut.difftest_logCtrl_end.value = 0
    dut.difftest_uart_in_ch.value = -1
    print("Initialization complete.")

def reset(dut: DUTSimTop):
    dut.reset.value = 1
    dut.Step(10)
    dut.reset.value = 0
    print("Reset complete.")

def uart(dut: DUTSimTop):
    if dut.difftest_uart_out_valid.value:
        print(chr(dut.difftest_uart_out_ch.value), end='', flush=True)

def main():
    import sys
    dut = DUTSimTop()
    print("DUTSimTop initialized")
    if len(sys.argv) < 2:
        bin_file = "microbench-NutShell.bin"
    else:
        bin_file = sys.argv[1]
    if len(sys.argv) < 3:
        step_count = 1000000
    else:
        step_count = int(sys.argv[2])
    if not load_bin(dut, bin_file):
        print(f"Failed to load binary file: {bin_file}.")
        print("Usage: python example.py <binary_file> [<step_count>]")
    dut.StepRis(lambda c: uart(dut))
    init(dut)
    reset(dut)
    step_count = 1000000
    print("Starting simulation with step count:", step_count)
    dut.Step(step_count)
    dut.Finish()
    print("Simulation finished.")

if __name__ == "__main__":
    main()
