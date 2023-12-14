#Makefile for {{__TOP_MODULE_NAME__}}

export SIMULATOR = {{__SIMULATOR__}}
export TRACE := {{__TRACE__}}
export PROJECT := {{__TOP_MODULE_NAME__}}
export TARGET := UT_{{__TOP_MODULE_NAME__}}
export VERBOSE := {{__VERBOSE__}}
export EXAMPLE := {{__EXAMPLE__}}
export TLANG := {{__TARGET_LANGUAGE__}}

all: release 

compile:
	cmake . -Bbuild -DSIMULATOR=$(SIMULATOR) -DTRACE=$(TRACE) -DPROJECT=$(PROJECT)
	cmake --build build --config Release --parallel `nproc`

release: compile
	@cp -r build/${TARGET} .
	@cp dut_base.hpp ${TARGET}/dut_base.hpp
	@make -f mk/${TLANG}.mk

ifneq ($(VERBOSE), OFF)
	cp *.v *.sv ${TARGET}/
else
# remove all files except UT_*
	rm -rf $(shell ls |grep -v "UT_")
endif

clean: 