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

    print("reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst.W()))
    dut.reset.value = 1
    rst.Set(1)
    print(rst.W())
    print("reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst.W()))
    dut.Step(1)
    rst.value = 1
    print("reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst.W()))
    rst.value = 1
    dut.reset.value = 0
    print("reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst.W()))
    dut.Step(1)
    print("reset from dpi: {}, reset from xcfg: {}".format(dut.reset.value, rst.W()))

    dut.Finish()
