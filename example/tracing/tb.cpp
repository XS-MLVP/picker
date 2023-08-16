#include <memory>
#include <verilated.h>
#include "Trace.h"

double sc_time_stamp() { return 0; }

int main(int argc, char** argv) {
    if (false && argc && argv) {}
    Verilated::mkdir("logs");
    const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
    contextp->debug(0);
    contextp->randReset(2);
    contextp->traceEverOn(true);
    contextp->commandArgs(argc, argv);
    const std::unique_ptr<Trace> top{new Trace{contextp.get(), "TOP"}};
    // Set Vtop's input signals
    top->reset_l = !0;
    top->clk = 0;
    top->in_small = 1;
    top->in_quad = 0x1234;
    top->in_wide[0] = 0x11111111;
    top->in_wide[1] = 0x22222222;
    top->in_wide[2] = 0x3;
    while (!contextp->gotFinish()) {
        contextp->timeInc(1);  // 1 timeprecision period passes...
        top->clk = !top->clk;
        if (!top->clk) {
            if (contextp->time() > 1 && contextp->time() < 10) {
                top->reset_l = !1;  // Assert reset
            } else {
                top->reset_l = !0;  // Deassert reset
            }
            top->in_quad += 0x12;
        }
        top->eval();
        VL_PRINTF("[%" PRId64 "] clk=%x rstl=%x iquad=%" PRIx64 " -> oquad=%" PRIx64
                  " owide=%x_%08x_%08x\n",
                  contextp->time(), top->clk, top->reset_l, top->in_quad, top->out_quad,
                  top->out_wide[2], top->out_wide[1], top->out_wide[0]);
    }
    top->final();
#if VM_COVERAGE
    Verilated::mkdir("logs");
    contextp->coveragep()->write("logs/coverage.dat");
#endif
    return 0;
}
