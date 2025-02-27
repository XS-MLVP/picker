#Makefile for {{__TOP_MODULE_NAME__}}

export SIMULATOR = {{__SIMULATOR__}}
export TRACE := {{__TRACE__}}
export PROJECT := {{__TOP_MODULE_NAME__}}
export TARGET := UT_{{__TOP_MODULE_NAME__}}
export VERBOSE := {{__VERBOSE__}}
export EXAMPLE := {{__EXAMPLE__}}
export TLANG := {{__TARGET_LANGUAGE__}}
export SIMULATOR_FLAGS := {{__VFLAG__}}
export CFLAGS := {{__CFLAG__}}
export COVERAGE := {{__COVERAGE__}}
export CHECKPOINTS := {{__CHECKPOINTS__}}
export VPI := {{__VPI__}}

ifeq ($(SIMULATOR), verilator)
V_ROOT=$(shell $(shell echo $$SHELL) -c  "verilator -V | grep VERILATOR_ROOT | awk '{print \$$3}' | head -n1")
endif

all: release 

compile:
	mkdir -p build
	cp pli.tab build/pli.tab
	cmake . -Bbuild -DSIMULATOR=$(SIMULATOR) -DTRACE=$(TRACE) -DPROJECT=$(PROJECT) -DCMAKE_BUILD_PARALLEL=`nproc`
	cmake --build build --config release --parallel `nproc`

ifeq ($(SIMULATOR), verilator)
# file exsit check with Makefile
ifeq ($(wildcard build/DPI{{__TOP_MODULE_NAME__}}/V{{__TOP_MODULE_NAME__}}___024root.h),)
	@rm -rf mem_direct
	@{{__GENERATOR_PICKER_PATH__}} export build/DPI{{__TOP_MODULE_NAME__}}/V{{__TOP_MODULE_NAME__}}___024root.h \
		--source_dir {{__GENERATOR_TEMPLATE_PATH__}}/mem_direct --target_dir ./mem_direct \
		--source_module_name {{__TOP_MODULE_NAME__}}
endif
	@cd mem_direct && g++ gen_addr.cpp -o export.bin \
		-I ../build/DPI{{__TOP_MODULE_NAME__}} \
		-I ${V_ROOT}/include \
		-I ${V_ROOT}/include/vltstd/ \
		-L../build -lDPI{{__TOP_MODULE_NAME__}} -lpthread
	@./mem_direct/export.bin > mem_direct/{{__TOP_MODULE_NAME__}}_offset.yaml
endif

release: compile
	@cp -r build/${TARGET} .
	@cp dut_base.hpp ${TARGET}/dut_base.hpp
	@cp mem_direct/{{__TOP_MODULE_NAME__}}_offset.yaml ${TARGET}/{{__TOP_MODULE_NAME__}}_offset.yaml
	@make -f mk/${TLANG}.mk

ifeq ($(VERBOSE), OFF) # if VERBOSE is OFF
	@rm -rf build cmake cpp mk python java scala golang lua CMakeLists.txt dut_base.* 2>/dev/null|| true
	@rm -rf ${TARGET}.cpp filelist.f *.fst 2>/dev/null|| true
	@rm -rf mem_direct
	@mv ${TARGET}/*.v ${TARGET}/*.sv . 2>/dev/null|| true
	@cd ${TARGET} && make purge
ifeq ($(COVERAGE), OFF)
	@rm Makefile
	@rm *.v *.sv
endif
endif

coverage:
ifeq ($(COVERAGE), ON)
ifeq ($(SIMULATOR), verilator)
	@rm -rf coverage/
	@verilator_coverage -write-info coverage.info ./V${PROJECT}_coverage.dat
	@genhtml coverage.info --output-directory coverage
else
# TODO: add vcs?
	@echo $(SIMULATOR) TBD
endif
else
	@echo coverage is not on
endif

clean: 