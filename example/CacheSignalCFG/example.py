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

    print(dut.xcfg.GetSignalNames("CacheCFG_top.clock"))

    clk = dut.xcfg.NewXData("CacheCFG_top.clock")
    rst = dut.xcfg.NewXData("CacheCFG_top.reset")
    # array_2 = dut.xcfg.NewXDataArray("CacheCFG_top.Cache.dataArray.ram.array_2")
    # print("Array_2 size: ", len(array_2))
    # print("Array_2: ", array_2[0])
    # print("Array_2: ", array_2)
    dut.reset.AsImmWrite()
    rst.AsImmWrite()


    print("Inital: reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst))
    dut.reset.value = 1
    print("set dpi, reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst))
    dut.Step(1)
    print("step, reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst))
    rst.value = 0
    print("set rst, reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst))
    dut.Step(1)
    print("step, reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst))

    dut.Finish()
