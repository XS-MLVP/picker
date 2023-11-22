#Makefile for {{__TOP_MODULE_NAME__}}

CPPFLAGS = {{__CPP_FLAGS__}} 
SIMULATOR = {{__SIMULATOR__}}


all: release


compile: clean libUT{{__TOP_MODULE_NAME__}}.so
ifeq ($(SIMULATOR), verilator)
	g++ $(CPPFLAGS) xdut_main.cpp -L. -lUT{{__TOP_MODULE_NAME__}} \
		-Wl,-rpath=. -o {{__TOP_MODULE_NAME__}}
else ifeq ($(SIMULATOR), vcs)
	g++ $(CPPFLAGS) xdut_main.cpp -L. -lUT{{__TOP_MODULE_NAME__}} \
		-Wl,-rpath=. -o {{__TOP_MODULE_NAME__}} \
		-no-pie  -Wl,--no-as-needed  -rdynamic  \
		-Wl,-whole-archive  -lvcsucli -Wl,-no-whole-archive  \
		-lzerosoft_rt_stubs -luclinative -lvirsim -lerrorinf -lsnpsmalloc -lvfs -lvcsnew -lsimprofile  -ldl -lc -lm -lpthread -lnuma \
		${VCS_HOME}/linux64/lib/vcs_tls.o ${VCS_HOME}/linux64/lib/vcs_save_restore_new.o 
endif

libUT{{__TOP_MODULE_NAME__}}.so: libDPI{{__TOP_MODULE_NAME__}}.so
	g++ $(CPPFLAGS) data.cpp port.cpp util.cpp UT{{__TOP_MODULE_NAME__}}.cpp \
		-L. -Wl,-rpath=./ -lDPI{{__TOP_MODULE_NAME__}} -shared -fPIC -o libUT{{__TOP_MODULE_NAME__}}.so

libDPI{{__TOP_MODULE_NAME__}}.so:
ifeq ($(SIMULATOR), verilator)
# Generate Verilaotr DPI library files
	verilator {{__VERILATOR_TRACE__}} --cc --prefix V{{__TOP_MODULE_NAME__}} --Mdir build/ {{__SIMULATOR_FLAGS__}} \
		-CFLAGS -fPIC  {{__TOP_MODULE_NAME__}}_top.sv {{__TOP_MODULE_NAME__}}.v --top-module {{__TOP_MODULE_NAME__}}_top 
	cp cpp.mk build/
	cd build/ && make -f cpp.mk -j`nproc`
	cp build/libV{{__TOP_MODULE_NAME__}}.so ./libDPI{{__TOP_MODULE_NAME__}}.so
	cp build/V{{__TOP_MODULE_NAME__}}.h ./
	cp build/V{{__TOP_MODULE_NAME__}}__Dpi.h ./
else ifeq ($(SIMULATOR), vcs)
	vcs -e VcsMain -slave {{__VCS_TRACE__}} -sverilog -lca -l compile.log \
		{{__TOP_MODULE_NAME__}}_top.sv {{__TOP_MODULE_NAME__}}.v  -o libDPI{{__TOP_MODULE_NAME__}}.so \
		+modelsave -LDFLAGS "-shared" -debug  \
		{{__SIMULATOR_FLAGS__}}	\
		-P ${VERDI_HOME}/share/PLI/VCS/LINUX64/novas.tab ${VERDI_HOME}/share/PLI/VCS/LINUX64/pli.a 
endif
# Generate  Debugging Information
	objdump -D ./libDPI{{__TOP_MODULE_NAME__}}.so > ./libDPI{{__TOP_MODULE_NAME__}}.S
# Generate CXX Wrapper files
	cat dut_base.hpp > UT{{__TOP_MODULE_NAME__}}.hpp
	tail -n +3 dut.hpp >> UT{{__TOP_MODULE_NAME__}}.hpp
	echo '#include "UT{{__TOP_MODULE_NAME__}}.hpp"' > UT{{__TOP_MODULE_NAME__}}.cpp
	tail -n +2 dut_base.cpp >> UT{{__TOP_MODULE_NAME__}}.cpp
	tail -n +2 dut.cpp >> UT{{__TOP_MODULE_NAME__}}.cpp

simulate: compile
ifeq ($(SIMULATOR), vcs)
# VCS MUST HAVE EXECUTABLE_NAME.daidir as DATA SOURCE
	ln -s libDPI{{__TOP_MODULE_NAME__}}.so.daidir {{__TOP_MODULE_NAME__}}.daidir
endif
	./{{__TOP_MODULE_NAME__}}

release: libDPI{{__TOP_MODULE_NAME__}}.so
	mkdir UT{{__TOP_MODULE_NAME__}}
	cp libDPI{{__TOP_MODULE_NAME__}}.so UT{{__TOP_MODULE_NAME__}}
	cp UT{{__TOP_MODULE_NAME__}}.hpp UT{{__TOP_MODULE_NAME__}}
	cp UT{{__TOP_MODULE_NAME__}}.cpp UT{{__TOP_MODULE_NAME__}}
	cp *.v UT{{__TOP_MODULE_NAME__}}
	cp *.sv UT{{__TOP_MODULE_NAME__}}
ifeq ($(SIMULATOR), verilator)
	cp V{{__TOP_MODULE_NAME__}}*.h UT{{__TOP_MODULE_NAME__}}
else ifeq ($(SIMULATOR), vcs)
	cp vc_hdrs.h UT{{__TOP_MODULE_NAME__}}
	cp -r {{__TOP_MODULE_NAME__}}.daidir UT{{__TOP_MODULE_NAME__}}
endif

clean: 
	rm -rf build lib{{__TOP_MODULE_NAME__}}.* libUT{{__TOP_MODULE_NAME__}}.* \
		libV{{__TOP_MODULE_NAME__}}.* V{{__TOP_MODULE_NAME__}}* \
		{{__TOP_MODULE_NAME__}}  UT{{__TOP_MODULE_NAME__}}*
