# Common Makefile for picker pack testing
# This Makefile works with generated packages (without -e flag)
# Copy this to pack_output directory and run: make clean comp copy_xspcomm run

PYTHON_INCLUDE:=$(firstword $(shell python3-config --includes))
XSP_COMM_INCLUDE:=$(shell picker --show_xcom_lib_location_cpp | awk '/Include:/ {print $$2}')
XSP_PYTHON:=$(shell picker --show_xcom_lib_location_python | sed -n '$$p')
# Auto-detect package directory name (exclude uvmpy, build, and hidden dirs)
PKG_DIR := $(shell find . -maxdepth 1 -type d ! -name "." ! -name ".." ! -name "uvmpy" ! -name "build" ! -name ".*" -printf "%f\n" | grep -v "^$$" | head -1)
BUILD_DIR := $(PKG_DIR)/build

# Validate package directory exists
ifeq ($(PKG_DIR),)
$(error No package directory found. Run picker pack first.)
endif

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
	-debug_access+all \
	-kdb \
	-CFLAGS -DVCS ${UVM_HOME}/src/dpi/uvm_dpi.cc \
	-e VcsMain sv_main ${VCS_HOME}/linux64/lib/vcs_tls.o -slave

.PHONY: all clean comp comp_simv copy_xspcomm run run_simv help

# Default target
all: clean comp copy_xspcomm run

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(PKG_DIR)/build $(PKG_DIR)/work $(PKG_DIR)/.vlogan*
	rm -rf $(PKG_DIR)/_tlm_pbsb.so* $(PKG_DIR)/simv*
	rm -rf $(PKG_DIR)/vcs.log $(PKG_DIR)/AN* $(PKG_DIR)/*.log
	rm -rf $(PKG_DIR)/*.log.cmp $(PKG_DIR)/*.vpd $(PKG_DIR)/DVE*
	rm -rf $(PKG_DIR)/*.fsdb $(PKG_DIR)/novas* $(PKG_DIR)/verdiLog
	rm -rf __pycache__ $(PKG_DIR)/__pycache__
	@echo "Clean complete."

# Compile VCS simulation
comp:
	@echo "Compiling with package directory: $(PKG_DIR)"
	@mkdir -p $(BUILD_DIR)
	swig -D'MODULE_NAME="tlm_pbsb"' -python -c++ -DUSE_VCS -I${XSP_COMM_INCLUDE} \
		-outdir $(PKG_DIR) -o $(BUILD_DIR)/tlmps.cpp \
		${XSP_COMM_INCLUDE}/xspcomm/python_tlm_pbsb.i
	$(SYSCAN) -Mdir=$(BUILD_DIR) \
			${XSP_COMM_INCLUDE}/xspcomm/tlm_pbsb.cpp $(BUILD_DIR)/tlmps.cpp
	@if [ -f example.sv ]; then \
		echo "Found example.sv"; \
		$(VLOGAN) +incdir+$(PKG_DIR) example.sv -Mdir=$(BUILD_DIR); \
	else \
		echo "Error: No UVM testbench file found (example.sv)!"; \
		exit 1; \
	fi
	$(VCS_ELAB) -o $(PKG_DIR)/_tlm_pbsb.so -Mdir=$(BUILD_DIR)
	@echo "Cleaning build artifacts..."
	# @rm -rf $(BUILD_DIR)
	@rm -f $(PKG_DIR)/vc_hdrs.h $(PKG_DIR)/ucli.key
	@echo "✓ Compilation complete"

# Copy xspcomm Python package
copy_xspcomm:
	@echo "Copying xspcomm package..."
	@cp -r ${XSP_PYTHON}/xspcomm $(PKG_DIR)/xspcomm
	@echo "✓ xspcomm copied"

# Run test (auto-detect test file)
run:
	@echo ""
	@echo "========================================================================"
	@echo "                          Running Test                                  "
	@echo "========================================================================"
	@echo ""
	@if [ -f example.py ]; then \
		echo "Running example.py"; \
		cd $(PKG_DIR) && LD_PRELOAD=./_tlm_pbsb.so python3 ../example.py; \
	else \
		echo "Error: No Python test file found (example.py)!"; \
		exit 1; \
	fi
