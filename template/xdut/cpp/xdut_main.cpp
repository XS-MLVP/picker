#include <bits/stdc++.h>
#include "dut.hpp"

int main(int argc, char **argv)
{
	UT{{__TOP_MODULE_NAME__}} dut(argc, argv);
	dut.io_in_req_bits_addr = "0x114514";
	dut.clock = 0;
	dut.reset = 1;
	dut.step();
	printf("io_req_ready = %s\n", dut.io_in_req_bits_addr.String().c_str());
	printf("io_req_ready = %s\n", dut.io_in_req_ready.String().c_str());
	dut.step();
}