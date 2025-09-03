CURRENT_DIR := $(shell pwd)
.PHONY: all test compile clean purge copy_xspcomm

all: test clean
	@echo "All complete"

test: compile
	@cp build/UT{{__TOP_MODULE_NAME__}}_example .
	@rm -rf build
	@echo "--------------Compile Success Test Step DUT--------------------"
	@./UT{{__TOP_MODULE_NAME__}}_example && rm -rf *.fst *.vcd *.fsdb *.log *.key 2>/dev/null || true
	@echo "--------------------------------------------------------------"

# copy xspcomm lib
copy_xspcomm_lib={{__COPY_XSPCOMM_LIB__}}
ifneq ($(copy_xspcomm_lib), true)
copy_xspcomm:
	@echo "Ignore copy xspcomm lib and include"
else
copy_xspcomm:
	@echo "Try copy xspcomm lib and include"
	@cp {{__XSPCOMM_LIB__}}/libxspcomm.so .
	@cp {{__XSPCOMM_INCLUDE__}} -r .
endif

compile: copy_xspcomm
	@echo "start build wrapper at $(shell date)"
	@cmake . -Bbuild
	@cmake --build build --config Release --parallel `nproc`
	@echo "end build wrapper at $(shell date)"

clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key *.dat build

purge: clean
	@mv ./* ../ || true
	@echo "purge complete"
	@rm -rf $(CURRENT_DIR)
