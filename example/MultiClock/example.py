try:
    from UT_multi_clock import *
except:
    try:
        from MultiClock import *
    except:
        from __init__ import *


from xspcomm import XClock


def test_multi_clock():
    dut = DUTmulti_clock()
    main_clock = XClock(dut.dut.simStep)
    
    clk1, clk2, clk3 = XClock(lambda x: 0), XClock(lambda x: 0), XClock(lambda x: 0)
    clk4, clk5, clk6 = XClock(lambda x: 0), XClock(lambda x: 0), XClock(lambda x: 0)
    
    clk1.Add(dut.xport.SelectPins(["reg1"])).Add(dut.clk1.xdata)
    clk2.Add(dut.xport.SelectPins(["reg2"])).Add(dut.clk2.xdata)
    clk3.Add(dut.xport.SelectPins(["reg3"])).Add(dut.clk3.xdata)
    clk4.Add(dut.xport.SelectPins(["reg4"])).Add(dut.clk4.xdata)
    clk5.Add(dut.xport.SelectPins(["reg5"])).Add(dut.clk5.xdata)
    clk6.Add(dut.xport.SelectPins(["reg6"])).Add(dut.clk6.xdata)
    
    main_clock.FreqDivWith(1, clk1)
    main_clock.FreqDivWith(2, clk2)
    main_clock.FreqDivWith(3, clk3)
    main_clock.FreqDivWith(1, clk4, -1)
    main_clock.FreqDivWith(2, clk5, 1)
    main_clock.FreqDivWith(3, clk6, 2)

    main_clock.Step(100)
    dut.Finish()
    print("main_clock:", main_clock.clk)
    print("clk1:", clk1.clk, "clk2:", clk2.clk, "clk3:", clk3.clk)
    print("clk4:", clk4.clk, "clk5:", clk5.clk, "clk6:", clk6.clk)
    print("Test multi clock completed.")
    print(" By default, waveform file is deleted after the simulation is finished")
    print(" If you want to keep the waveform file, please run `python3 example.py` in the directory: picker_out/MultiClock")


if __name__ == "__main__":
    test_multi_clock()
