CPPFLAGS = -I {{verilator_include}} {{cpp_flages}}

defualt: *.cpp {{cpplib_filename}}
	g++ $(CPPFLAGS) $^ -o {{dut_name}}
