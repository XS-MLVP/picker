#include "UT_Adder.hpp"

int64_t random_int64()
{
    static std::random_device rd;
    static std::mt19937_64 generator(rd());
    static std::uniform_int_distribution<int64_t> distribution(INT64_MIN, INT64_MAX);
    return distribution(generator);
}

int main()
{
    UTAdder *dut = new UTAdder();
    dut->Step(1);
    printf("Initialized UTAdder\n");

    struct input_t {
        uint64_t a;
        uint64_t b;
        uint64_t cin;
    };

    struct output_t {
        uint64_t sum;
        uint64_t cout;
    };

    for (int c = 0; c < 114514; c++) {
        input_t i;
        output_t o_dut, o_ref;

        i.a   = random_int64();
        i.b   = random_int64();
        i.cin = random_int64() & 1;

        auto dut_cal = [&]() {
            dut->a   = i.a;
            dut->b   = i.b;
            dut->cin = i.cin;
            dut->Step(1);
            o_dut.sum  = (uint64_t)dut->sum;
            o_dut.cout = (uint64_t)dut->cout;
        };

        auto ref_cal = [&]() {
            uint64_t sum = i.a + i.b;
            bool carry   = sum < i.a;

            sum += i.cin;
            carry = carry || sum < i.cin;

            o_ref.sum  = sum;
            o_ref.cout = carry;
        };

        dut_cal();
        ref_cal();
        printf("[cycle %lu] a=0x%lx, b=0x%lx, cin=0x%lx\n", dut->xclock.clk, i.a, i.b, i.cin);
        printf("DUT: sum=0x%lx, cout=0x%lx\n", o_dut.sum, o_dut.cout);
        printf("REF: sum=0x%lx, cout=0x%lx\n", o_ref.sum, o_ref.cout);
        Assert(o_dut.sum == o_ref.sum, "sum mismatch");
    }

    dut->Finish();
    printf("Test Passed, destory UTAdder\n");
    return 0;
}