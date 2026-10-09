try:
    from UT_dpi_dollar import *
except ImportError:
    from dpi_dollar import *


if __name__ == "__main__":
    dut = DUTdpi_dollar()
    try:
        input_signal = dut.pickerdpix696e2431
        output_signal = dut.pickerdpix6f75742431
        internal_signal = dut.pickerdpix6470695f646f6c6c61722e7369672431
        input_signal.value = 1
        dut.Step(1)
        assert output_signal.value == 1
        assert internal_signal.value == 1
    finally:
        dut.Finish()
    print("DPI_INTERNAL_DOLLAR_OK")
