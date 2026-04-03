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
    print("Pins startwith(CacheCFG_top.clock): ", dut.xcfg.GetSignalNames("CacheCFG_top.clock"))
    clk = dut.GetInternalSignal("CacheCFG_top.clock")
    rst = dut.GetInternalSignal("CacheCFG_top.reset")
    array_2 = dut.GetInternalSignal("CacheCFG_top.Cache.dataArray.ram.array_2", is_array=True)
    direct_signal = dut.GetInternalSignal("CacheCFG_top.Cache.arb.io_in_1_bits_cmd")
    direct_source = dut.GetInternalSignal("CacheCFG_top.io_in_req_bits_cmd")
    projection_signal = dut.GetInternalSignal("CacheCFG_top.Cache.arb.grant_1")
    expr_signal = dut.GetInternalSignal("CacheCFG_top.Cache.arb.io_out_bits_cmd")

    assert clk is not None
    assert rst is not None
    assert direct_signal is not None
    assert direct_source is not None
    assert projection_signal is not None
    assert expr_signal is not None

    print("Array_2 size: ", len(array_2))
    print("Array_2  [0]: ", array_2[0])
    print("Read direct signal      : ", direct_signal.value)
    print("Read direct source      : ", direct_source.value)
    print("Read projection signal  : ", projection_signal.value)
    print("Read expr signal        : ", expr_signal.value)
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
    direct_signal.AsImmWrite()
    direct_source.AsImmWrite()
    direct_signal.value = 0xA
    print("Write direct alias/source       : {} / {}".format(direct_signal.value, direct_source.value))
    print("Read direct signal post-step     : ", direct_signal.value)
    print("Read direct source post-step     : ", direct_source.value)
    print("Read projection signal post-step : ", projection_signal.value)
    print("Read expr signal post-step       : ", expr_signal.value)
    print("All complete")
    dut.Finish()
