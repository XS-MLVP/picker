PYTHON_INCLUDE:="/usr/include/python3.6m"
XSP_COMM_INCLUDE:="{{__XSPCOMM_INCLUDE__}}"
ifndef UVMC_HOME
$(error UVMC_HOME is not set)
endif
ifndef VCS_HOME
$(error VCS_HOME is not set)
endif
ifndef UVM_HOME
$(error UVM_HOME is not set)
endif
ifndef XSP_COMM_INCLUDE
$(error XSP_COMM_INCLUDE is not set)
endif
ifndef PYTHON_INCLUDE
$(error PYTHON_INCLUDE is not set)
endif


VERBOSE=+UVM_VERBOSITY=UVM_MEDIUM

SYSCAN  = syscan -cpp g++ -cc gcc -full64 -cflags -g -cflags -DVCS -cflags -I. \
                -tlm2 -cflags -I${VCS_HOME}/include/systemc232/tlm_utils \
                -cflags -I${UVMC_HOME}/src/connect/sc ${UVMC_HOME}/src/connect/sc/uvmc.cpp

VLOGAN  = vlogan -q \
         -sverilog +incdir+${UVM_HOME}/src ${UVM_HOME}/src/uvm_pkg.sv \
        +incdir+${UVMC_HOME}/src/connect/sv ${UVMC_HOME}/src/connect/sv/uvmc_pkg.sv \
         -timescale=1ns/1ps \

VCS_ELAB = vcs -sysc=deltasync -lca \
        -full64 -sysc -cpp g++ -cc gcc \
        -timescale=1ns/1ps \
        -CFLAGS -DVCS ${UVM_HOME}/src/dpi/uvm_dpi.cc

VCS    = vcs -q -sysc=deltasync -lca -full64 \
        -sverilog +incdir+${UVM_HOME}/src ${UVM_HOME}/src/uvm_pkg.sv \
        +incdir+${UVMC_HOME}/src/connect/sv ${UVMC_HOME}/src/connect/sv/uvmc_pkg.sv \
        -timescale=1ns/1ps \
        -sysc -cpp g++ -cc gcc \
        -CFLAGS -DVCS ${UVM_HOME}/src/dpi/uvm_dpi.cc

all: 
	make clean comp copy_xspcomm run 

clean:
	rm -rf simv* work csrc ucli.key vc_hdrs.h vcs.log AN* *.log *.log.cmp *.vpd DVE* .vlogan* _tlm_pbsb* tlm_pbsb.py  tlmps.cpp  tlmps.h __pycache__


comp:
	swig -D'MODULE_NAME="tlm_pbsb"' -python -c++ -DUSE_VCS -I${XSP_COMM_INCLUDE} -o tlmps.cpp ${XSP_COMM_INCLUDE}/xspcomm/python_tlm_pbsb.i
	$(SYSCAN) -full64 -cflags -DUSE_VCS -cflags -I${PYTHON_INCLUDE} -cflags -I${XSP_COMM_INCLUDE} ${XSP_COMM_INCLUDE}/xspcomm/tlm_pbsb.cpp tlmps.cpp
	$(VLOGAN) -full64 +incdir+common example_uvm.sv +define+UVM_OBJECT_MUST_HAVE_CONSTRUCTOR
	$(VCS_ELAB) -e VcsMain -full64 sv_main ${VCS_HOME}/linux64/lib/vcs_tls.o -slave
	mv simv _tlm_pbsb.so
	mv simv.daidir _tlm_pbsb.so.daidir
copy_xspcomm:
	@echo "Try copy python module: xspcomm"
	@cp -r {{__XSPCOMM_PYTHON__}} xspcomm
run:	
	LD_PRELOAD=./_tlm_pbsb.so python3 example_python.py