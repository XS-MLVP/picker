try:
    from UT_MultiFileRTLTop import *
except:
    try:
        from MultiFileRTLTop import *
    except:
        from __init__ import *


if __name__ == "__main__":
    dut = DUTMultiFileRTLTop()

    test_vectors = [
        0x0,
        0x1,
        0x5A,
        0x123,
        0xABC,
        0xFFF,
    ]

    for idx, value in enumerate(test_vectors):
        dut.a.value = value
        dut.Step(1)
        print(f"case {idx}: a=0x{value:x}, b=0x{dut.b.value:x}")
        assert dut.b.value == value, "output mismatch"

    print("Test Passed, destroy UTMultiFileRTLTop")
    dut.Finish()
