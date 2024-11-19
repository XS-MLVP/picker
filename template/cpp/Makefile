CURRENT_DIR := $(shell pwd)

all: test clean
	@echo "All complete"

test: compile
	@cp build/UT{{__TOP_MODULE_NAME__}}_example .
	@rm -rf build
	@echo "--------------Comple Success Test Step DUT--------------------"
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
	@cmake . -Bbuild
	@cmake --build build --config Release --parallel `nproc`

clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key *.dat build

purge: clean
	@mv ./* ../ || true
	@echo "purge complete"
	@rm -rf $(CURRENT_DIR)
