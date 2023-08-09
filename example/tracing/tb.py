import sys
import os

def test_trace():
    import Trace as t
    t.v_mkdir("logs")
    ctx = t.VerilatedContext()
    ctx.debug(0)
    ctx.randReset(2)
    ctx.traceEverOn(True)
    ctx.commandArgs(" ".join(sys.argv))

    top = t.Trace(ctx, "TOP")
    top.reset_l = 1
    top.clk = 0
    top.in_small = 1
    top.in_quad = 0x1234

    top.in_wide[0] = 0x11111111
    top.in_wide[1] = 0x22222222
    top.in_wide[2] = 0x3

    while not ctx.gotFinish():
        ctx.timeInc(1)
        top.clk = 0 if top.clk > 0 else 1
        if top.clk == 0:
            if ctx.time() > 1 and ctx.time() < 10:
                top.reset_l = 0
            else:
                top.reset_l = 1
            top.in_quad += 0x12
        top.eval()
        print("[%ld] clk=%x rstl=%x iquad=%lx -> oquad=%lx owide=%x_%08x_%08x" % (
                  ctx.time(), top.clk, top.reset_l, top.in_quad, top.out_quad,
                  top.out_wide[2], top.out_wide[1], top.out_wide[0]))
    
    top.final()
    
    ctx.coveragep_write("logs/coverage.dat")
    del top
    del ctx
    
    print("\n-- COVERAGE ----------------")
    os.system("verilator_coverage --annotate logs/annotated logs/coverage.dat")
    print("\n-- DONE --------------------")
    print("To see waveforms, open vlt_dump.vcd in a waveform viewer")


def usage():
    print("usage %s <dut_lib_dir> [options]" % sys.argv[0])


def test():
    if len(sys.argv) < 2:
        usage()
        return
    im_dir = sys.argv.pop(1)
    if not os.path.exists(im_dir):
        usage()
        print("dut_lib_dir(%s) not find"%im_dir)
        return    
    sys.path.insert(0, im_dir)
    test_trace()


if __name__ == "__main__":
    test()    
