#Makefile for {{__TOP_MODULE_NAME__}}

CPPFLAGS = -I /usr/local/share/verilator/include -I /usr/local//share/verilator/include/vltstd/ -DVL_DEBUG=1

all: release


compile: clean lib{{__TOP_MODULE_NAME__}}.so 
	g++ $(CPPFLAGS) xdut_main.cpp -L. -l{{__TOP_MODULE_NAME__}} \
		-Wl,-rpath=. -o {{__TOP_MODULE_NAME__}}


lib{{__TOP_MODULE_NAME__}}.so: libV{{__TOP_MODULE_NAME__}}.a
	g++ $(CPPFLAGS) data.cpp dut_base.cpp dut.cpp \
		-L. -lV{{__TOP_MODULE_NAME__}} -shared -fPIC -o lib{{__TOP_MODULE_NAME__}}.so	


libV{{__TOP_MODULE_NAME__}}.a:
	verilator --cc --prefix V{{__TOP_MODULE_NAME__}} --Mdir build/ -CFLAGS -fPIC  {{__TOP_MODULE_NAME__}}_top.sv {{__TOP_MODULE_NAME__}}.v --top-module {{__TOP_MODULE_NAME__}}_top 
	cp cpp.mk build/
	cd build/ && make -f cpp.mk
	cp build/libV{{__TOP_MODULE_NAME__}}.a ./
	cp build/V{{__TOP_MODULE_NAME__}}.h ./
	cp build/V{{__TOP_MODULE_NAME__}}__Dpi.h ./
	objdump -D libV{{__TOP_MODULE_NAME__}}.a > libV{{__TOP_MODULE_NAME__}}.S

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
