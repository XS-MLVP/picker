import sys
import os

Yes = 1

def test_protect_lib():
    import Protect as p
    ctx = p.VerilatedContext()
    ctx.debug(0)
    ctx.randReset(2)
    ctx.commandArgs(" ".join(sys.argv))
    top = p.Protect(ctx)

    ts = ctx.if_compile_trace()
    print("comple with trace: %s"%ts)

    if ts and ctx.commandArgsPlusMatch("trace") == "+trace":
        ctx.traceEverOn(Yes)
        print("Enabling waves into logs/vlt_dump.vcd...")
        tfp = p.VerilatedVcdC()
        top.trace(tfp, 99)
        p.v_mkdir("logs")
        tfp.open("logs/vlt_dump.vcd")
    
    top.clk = 0
    while not ctx.gotFinish():
        ctx.timeInc(1)
        top.clk = 1 if top.clk < 1 else 0
        top.eval()
        if ts:
            tfp.dump(ctx.time())
            
    top.final()
    del top
    if ts:
        tfp.close()
        del tfp
    del ctx


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
    test_protect_lib()


if __name__ == "__main__":
    test()    
