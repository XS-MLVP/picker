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

all: release 

compile:
	cmake . -Bbuild -DSIMULATOR=$(SIMULATOR) -DTRACE=$(TRACE) -DPROJECT=$(PROJECT) \
		-DSIMULATOR_FLAGS=$(SIMULATOR_FLAGS) -DCFLAGS=$(CFLAGS) -DCOVERAGE=$(COVERAGE)
	cmake --build build --config Debug --parallel `nproc`

release: compile
	@cp -r build/${TARGET} .
	@cp dut_base.hpp ${TARGET}/dut_base.hpp
	@make -f mk/${TLANG}.mk

ifeq ($(VERBOSE), OFF) # if VERBOSE is OFF

ifeq ($(COVERAGE), OFF)
	@rm Makefile
	@rm *.v *.sv
endif

	@rm -rf build cmake cpp mk python CMakeLists.txt dut_base.*
	@rm -rf ${TARGET}.cpp
	@mv ${TARGET}/*.v ${TARGET}/*.sv . || true
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