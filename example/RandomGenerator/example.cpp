#include "UT_RandomGenerator.hpp"

int64_t random_int64()
{
    static std::random_device rd;
    static std::mt19937_64 generator(rd());
    static std::uniform_int_distribution<int64_t> distribution(INT64_MIN, INT64_MAX);
    return distribution(generator);
}

int main()
{
    UTRandomGenerator *dut = new UTRandomGenerator();
    unsigned short seed    = random_int64() & 0xffff;
    printf("seed = 0x%x\n", seed);
    dut->InitClock(dut->clk);
    dut->Step(10);
    dut->reset = 1;
    dut->seed  = seed;
    dut->Step(1);
    dut->reset = 0;
    dut->Step(1);
    printf("Initialized UTRandomGenerator\n");

    struct output_t {
        uint64_t cout;
    };

    for (int c = 0; c < 114514; c++) {
        output_t o_dut, o_ref;

        auto dut_cal = [&]() {
            dut->Step(1);
            o_dut.cout = (unsigned short)dut->random_number;
        };

        // as lfsr
        auto ref_cal = [&]() {
            seed       = (seed << 1) | ((seed >> 15) ^ (seed >> 14) & 1);
            o_ref.cout = seed;
        };

        dut_cal();
        ref_cal();
        printf("[cycle %lu] ", dut->xclock.clk);
        printf("DUT: cout=0x%lx , ", o_dut.cout);
        printf("REF: cout=0x%lx\n", o_ref.cout);
        Assert(o_dut.cout == o_ref.cout, "sum mismatch");
    }

    delete dut;
    printf("Test Passed, destory UTRandomGenerator\n");
    return 0;
}