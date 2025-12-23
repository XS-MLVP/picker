PYTHON_INCLUDE:=$(firstword $(shell python3-config --includes))
XSP_COMM_INCLUDE:=$(shell picker --show_xcom_lib_location_cpp | awk '/Include:/ {print $$2}')
XSP_PYTHON:=$(shell picker --show_xcom_lib_location_python | sed -n '$$p')
# Path management for optimized _pkg structure
# Project structure:
#   uvmpy/                       (CURDIR, project root,use -e to generate)
#   ├── <ProjectName>/           (PKG_DIR, Python package)
#   │   |── build/               (BUILD_DIR, build artifacts)
#   │   |── xagent.sv            (core sv file)
#   │   |── xagent.py            (core python files)
#   │   |── utils_pkg.sv         (serialize/deserialize macros)
#   │   └── ...                  (other files)
#   ├── example.py               (Python example)
#   ├── example.sv               (SystemVerilog example)
#   └── Makefile                 (this file)
TOP_DIR := $(CURDIR)
PKG_DIR := {{package_name}}
BUILD_DIR := $(PKG_DIR)/build

# Validate required environment variables
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
                -cflags -I${UVMC_HOME}/src/connect/sc ${UVMC_HOME}/src/connect/sc/uvmc.cpp \
                -cflags -DUSE_VCS -cflags ${PYTHON_INCLUDE} -cflags -I${XSP_COMM_INCLUDE}

VLOGAN  = vlogan -q -full64 \
         -sverilog +incdir+${UVM_HOME}/src ${UVM_HOME}/src/uvm_pkg.sv \
        +incdir+${UVMC_HOME}/src/connect/sv ${UVMC_HOME}/src/connect/sv/uvmc_pkg.sv \
         -timescale=1ns/1ps +define+UVM_OBJECT_MUST_HAVE_CONSTRUCTOR \

VCS_ELAB = vcs -sysc=deltasync -lca \
        -full64 -sysc -cpp g++ -cc gcc \
        -timescale=1ns/1ps \
        -P ${VERDI_HOME}/share/PLI/VCS/LINUX64/novas.tab \
        ${VERDI_HOME}/share/PLI/VCS/LINUX64/pli.a \
        -CFLAGS -DVCS ${UVM_HOME}/src/dpi/uvm_dpi.cc \
        -e VcsMain sv_main ${VCS_HOME}/linux64/lib/vcs_tls.o -slave

VCS    = vcs -q -sysc=deltasync -lca -full64 \
        -sverilog +incdir+${UVM_HOME}/src ${UVM_HOME}/src/uvm_pkg.sv \
        +incdir+${UVMC_HOME}/src/connect/sv ${UVMC_HOME}/src/connect/sv/uvmc_pkg.sv \
        -timescale=1ns/1ps \
        -sysc -cpp g++ -cc gcc \
        -CFLAGS -DVCS ${UVM_HOME}/src/dpi/uvm_dpi.cc

# Default target: compile and run
all: clean comp copy_xspcomm run

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(PKG_DIR)/build $(PKG_DIR)/work $(PKG_DIR)/.vlogan*
	rm -rf $(PKG_DIR)/_tlm_pbsb.so* $(PKG_DIR)/simv* 
	rm -rf $(PKG_DIR)/vcs.log $(PKG_DIR)/AN* $(PKG_DIR)/*.log 
	rm -rf $(PKG_DIR)/*.log.cmp $(PKG_DIR)/*.vpd $(PKG_DIR)/DVE*
	rm -rf __pycache__ $(PKG_DIR)/__pycache__
	@echo "Clean complete."

# Compile target (unified for both regular and DUT mode)
comp:
	@mkdir -p $(BUILD_DIR)
	swig -D'MODULE_NAME="tlm_pbsb"' -python -c++ -DUSE_VCS -I${XSP_COMM_INCLUDE} \
		-outdir $(PKG_DIR) -o $(BUILD_DIR)/tlmps.cpp \
		${XSP_COMM_INCLUDE}/xspcomm/python_tlm_pbsb.i
	$(SYSCAN) -Mdir=$(BUILD_DIR) \
		${XSP_COMM_INCLUDE}/xspcomm/tlm_pbsb.cpp $(BUILD_DIR)/tlmps.cpp
	@if [ -f example.sv ]; then \
		{% if rtl_file_path -%}
		$(VLOGAN) +incdir+$(PKG_DIR) {{rtl_file_path}} example.sv -Mdir=$(BUILD_DIR); \
		{% else -%}
		$(VLOGAN) +incdir+$(PKG_DIR) example.sv -Mdir=$(BUILD_DIR); \
		{% endif -%}
	else \
		echo "Error: No example UVM file found!"; \
		exit 1; \
	fi
	$(VCS_ELAB) -o $(PKG_DIR)/_tlm_pbsb.so -Mdir=$(BUILD_DIR)
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -f $(PKG_DIR)/vc_hdrs.h $(PKG_DIR)/ucli.key
	@echo "✓ Compilation complete"

# Copy xspcomm library to package directory
copy_xspcomm:
	@cp -r ${XSP_PYTHON}/xspcomm $(PKG_DIR)/xspcomm

# Run example (auto-detect DUT mode or non-DUT mode)
run:
	@if [ -f example.py ]; then \
		cd $(PKG_DIR) && LD_PRELOAD=./_tlm_pbsb.so python3 ../example.py; \
	else \
		echo "Error: No example file found (example.py)!"; \
		exit 1; \
	fi

# Help target
help:
	@echo "Available targets:"
	@echo "  make all          - Clean, compile, copy xspcomm, and run"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make comp         - Compile UVM testbench"
	@echo "  make copy_xspcomm - Copy xspcomm library to package directory"
	@echo "  make run          - Run the example (auto-detect DUT/non-DUT mode)"
	@echo "  make help         - Show this help message"

.PHONY: all clean comp copy_xspcomm run help
