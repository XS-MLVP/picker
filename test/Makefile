SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

.PHONY: all smoke export_python export_vpi_python pack_python clean

SCRIPT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))/scripts

all: smoke export_python export_vpi_python pack_python

smoke:
	bash "$(SCRIPT_DIR)/test_cli.sh"

export_python:
	bash "$(SCRIPT_DIR)/test_export_adder_python.sh"

export_vpi_python:
	bash "$(SCRIPT_DIR)/test_export_internal_vpi_python.sh"

pack_python:
	@if ! command -v vcs &> /dev/null; then \
		echo "Warning: vcs not found, skipping pack_python."; \
	else \
		bash "$(SCRIPT_DIR)/test_pack_python.sh"; \
	fi

clean:
	rm -rf picker_out_* pack_example
