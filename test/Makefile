SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

.PHONY: all preflight smoke export_python export_vpi_python pack_python export_java sv_build gsim_smoke clean

SCRIPT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/scripts

all: preflight smoke export_python export_vpi_python export_java sv_build gsim_smoke pack_python

# Ensure picker is built with python and java support before running tests
preflight:
	@ROOT_DIR=$$(git rev-parse --show-toplevel 2>/dev/null || pwd); \
	PICKER_BIN="$$ROOT_DIR/build/bin/picker"; \
	XCOMM_SRC_DIR="$$ROOT_DIR/dependence/xcomm"; \
	XCOMM_BUILD_DIR="$$ROOT_DIR/build/dependence/xcomm"; \
	need_rebuild=0; \
	if [ ! -x "$$PICKER_BIN" ]; then \
		echo "[preflight] picker not built"; \
		need_rebuild=1; \
	fi; \
	# python support: detect either top-level build path or xcomm standalone build path \
	if [ ! -d "$$XCOMM_BUILD_DIR/python/xspcomm" ] && [ ! -d "$$XCOMM_SRC_DIR/build/python" ]; then \
		echo "[preflight] xcomm python support missing"; \
		need_rebuild=1; \
	fi; \
	# java support: detect either top-level build path or xcomm standalone build path \
	if [ ! -f "$$XCOMM_BUILD_DIR/java/xspcomm-java.jar" ] && [ ! -f "$$XCOMM_SRC_DIR/build/java/xspcomm-java.jar" ]; then \
		echo "[preflight] xcomm java support missing"; \
		need_rebuild=1; \
	fi; \
	if [ "$$need_rebuild" -ne 0 ]; then \
		echo "[preflight] cleaning then rebuilding picker with BUILD_XSPCOMM_SWIG=python,java"; \
		if command -v nproc >/dev/null 2>&1; then NPROC=$$(nproc); else NPROC=2; fi; \
		$(MAKE) -C "$$ROOT_DIR" clean; \
		$(MAKE) -C "$$ROOT_DIR" BUILD_XSPCOMM_SWIG=python,java -j"$$NPROC"; \
	else \
		echo "[preflight] picker already has python+java support"; \
	fi

smoke:
	bash "$(SCRIPT_DIR)/test_cli.sh"

export_python: preflight
	bash "$(SCRIPT_DIR)/test_export_adder_python.sh"

export_vpi_python: preflight
	bash "$(SCRIPT_DIR)/test_export_internal_vpi_python.sh"

pack_python: preflight
	@if ! command -v vcs &> /dev/null; then \
		echo "Warning: vcs not found, skipping pack_python."; \
	else \
		bash "$(SCRIPT_DIR)/test_pack_python.sh"; \
	fi

export_java: preflight
	bash "$(SCRIPT_DIR)/test_export_adder_java.sh"

sv_build:
	bash "$(SCRIPT_DIR)/test_sv_verilator_build.sh"

gsim_smoke:
	bash "$(SCRIPT_DIR)/test_export_gsim_smoke.sh"

clean:
	rm -rf picker_out_* pack_example
