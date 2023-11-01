#Makefile for {{__TOP_MODULE_NAME__}}

CPPFLAGS = {{__CPP_FLAGS__}} 
SIMULATOR = {{__SIMULATOR__}}


all: release


compile: clean libUT{{__TOP_MODULE_NAME__}}.so
	g++ $(CPPFLAGS) xdut_main.cpp -L. -lUT{{__TOP_MODULE_NAME__}} \
		-Wl,-rpath=. -o {{__TOP_MODULE_NAME__}}


libUT{{__TOP_MODULE_NAME__}}.so: libDPI{{__TOP_MODULE_NAME__}}.so
	g++ $(CPPFLAGS) data.cpp UT{{__TOP_MODULE_NAME__}}.cpp \
		-L. -lDPI{{__TOP_MODULE_NAME__}} -shared -fPIC -o libUT{{__TOP_MODULE_NAME__}}.so	


libDPI{{__TOP_MODULE_NAME__}}.so:
ifeq ($(SIMULATOR), verilator)
# Generate Verilaotr DPI library files
	verilator {{__VERILAOTR_TRACE__}} --cc --prefix V{{__TOP_MODULE_NAME__}} --Mdir build/ -CFLAGS -fPIC  {{__TOP_MODULE_NAME__}}_top.sv {{__TOP_MODULE_NAME__}}.v --top-module {{__TOP_MODULE_NAME__}}_top 
	cp cpp.mk build/
# Compile Verilator DPI library
	cd build/ && make -f cpp.mk
# Copy Verilator DPI library files
	cp build/libV{{__TOP_MODULE_NAME__}}.so ./libDPI{{__TOP_MODULE_NAME__}}.so
	cp build/V{{__TOP_MODULE_NAME__}}.h ./
	cp build/V{{__TOP_MODULE_NAME__}}__Dpi.h ./
# Generate Verilator Debugging Information
	objdump -D ./libDPI{{__TOP_MODULE_NAME__}}.so > ./libDPI{{__TOP_MODULE_NAME__}}.S
else ifeq ($(SIMULATOR), vcs)
	vcs -full64 -sverilog -debug_all -f {{__TOP_MODULE_NAME__}}.f -l vcs.log
endif
# Generate CXX Wrapper files
	cat dut_base.hpp > UT{{__TOP_MODULE_NAME__}}.hpp
	tail -n +3 dut.hpp >> UT{{__TOP_MODULE_NAME__}}.hpp
	echo '#include "UT{{__TOP_MODULE_NAME__}}.hpp"' > UT{{__TOP_MODULE_NAME__}}.cpp
	tail -n +2 dut_base.cpp >> UT{{__TOP_MODULE_NAME__}}.cpp
	tail -n +2 dut.cpp >> UT{{__TOP_MODULE_NAME__}}.cpp

simulate: compile
	./{{__TOP_MODULE_NAME__}}

release: simulate
	mkdir UT{{__TOP_MODULE_NAME__}}
	cp {{__TOP_MODULE_NAME__}} UT{{__TOP_MODULE_NAME__}}
	cp libDPI{{__TOP_MODULE_NAME__}}.so UT{{__TOP_MODULE_NAME__}}
	cp UT{{__TOP_MODULE_NAME__}}.hpp UT{{__TOP_MODULE_NAME__}}
	cp UT{{__TOP_MODULE_NAME__}}.cpp UT{{__TOP_MODULE_NAME__}}
	cp data.* UT{{__TOP_MODULE_NAME__}}
	cp util.hpp UT{{__TOP_MODULE_NAME__}}
	cp *.v UT{{__TOP_MODULE_NAME__}}
	cp *.sv UT{{__TOP_MODULE_NAME__}}

clean: 
	rm -rf build lib{{__TOP_MODULE_NAME__}}.* libUT{{__TOP_MODULE_NAME__}}.* \
		libV{{__TOP_MODULE_NAME__}}.* V{{__TOP_MODULE_NAME__}}* \
		{{__TOP_MODULE_NAME__}}  UT{{__TOP_MODULE_NAME__}}*
