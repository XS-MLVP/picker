try:
    from UT_CacheCFG import *
except:
    try:
        from CacheCFG import *
    except:
        from __init__ import *


if __name__ == "__main__":
    dut = DUTCacheCFG()
    dut.InitClock("clock")
    print("Pins startwith(clock): ", dut.xcfg.GetSignalNames("clock"))
    clk = dut.GetInternalSignal("clock")
    rst = dut.GetInternalSignal("reset")
    array_2 = dut.GetInternalSignal("Cache.dataArray.ram.array_2", is_array=True)
    print("Array_2 size: ", len(array_2))
    print("Array_2  [0]: ", array_2[0])
    dut.reset.AsImmWrite()
    rst.AsImmWrite()
    print("Inital: reset from dpi : {}, reset from xcfg: {}".format(dut.reset.value, rst.value))
    dut.reset.value = 1
    print("set dpi, reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst.value))
    dut.Step(1)
    print("step, reset from dpi   : {}, reset from xcfg: {}".format(dut.reset.value, rst.value))
    rst.value = 0
    print("set rst, reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst.value))
    dut.Step(1)
    print("step, reset from dpi   : {}, reset from xcfg: {}".format(dut.reset.value, rst.value))
    dut.Finish()
