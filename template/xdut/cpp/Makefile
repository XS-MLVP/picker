#Makefile for {{__TOP_MODULE_NAME__}}

CPPFLAGS = {{__CPP_FLAGS__}} 
SIMULATOR = {{__SIMULATOR__}}


all: release


compile: clean lib{{__TOP_MODULE_NAME__}}.so 
	g++ $(CPPFLAGS) xdut_main.cpp -L. -l{{__TOP_MODULE_NAME__}} \
		-Wl,-rpath=. -o {{__TOP_MODULE_NAME__}}


lib{{__TOP_MODULE_NAME__}}.so: libV{{__TOP_MODULE_NAME__}}.a
	g++ $(CPPFLAGS) data.cpp dut_base.cpp dut.cpp \
		-L. -lV{{__TOP_MODULE_NAME__}} -shared -fPIC -o lib{{__TOP_MODULE_NAME__}}.so	


libV{{__TOP_MODULE_NAME__}}.a:
ifeq ($(SIMULATOR), verilator)
	verilator {{__VERILAOTR_TRACE__}} --cc --prefix V{{__TOP_MODULE_NAME__}} --Mdir build/ -CFLAGS -fPIC  {{__TOP_MODULE_NAME__}}_top.sv {{__TOP_MODULE_NAME__}}.v --top-module {{__TOP_MODULE_NAME__}}_top 
	cp cpp.mk build/
	cd build/ && make -f cpp.mk
	cp build/libV{{__TOP_MODULE_NAME__}}.a ./
	cp build/V{{__TOP_MODULE_NAME__}}.h ./
	cp build/V{{__TOP_MODULE_NAME__}}__Dpi.h ./
	objdump -D libV{{__TOP_MODULE_NAME__}}.a > libV{{__TOP_MODULE_NAME__}}.S
else ifeq ($(SIMULATOR), vcs)
	vcs -full64 -sverilog -debug_all -f {{__TOP_MODULE_NAME__}}.f -l vcs.log
endif

simulate: compile
	./{{__TOP_MODULE_NAME__}}

release: simulate
	mkdir release
	cp {{__TOP_MODULE_NAME__}}* release/
	cp lib{{__TOP_MODULE_NAME__}}.so release/
	cp *.hpp release/
	cp *.v release/
	cp *.sv release

clean: 
	rm -rf build lib{{__TOP_MODULE_NAME__}}.* libV{{__TOP_MODULE_NAME__}}.* {{__TOP_MODULE_NAME__}} V{{__TOP_MODULE_NAME__}}*
