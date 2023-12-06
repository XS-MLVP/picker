#Makefile for {{__TOP_MODULE_NAME__}}

SIMULATOR = {{__SIMULATOR__}}
TRACE := {{__TRACE__}}
PROJECT := {{__TOP_MODULE_NAME__}}
TARGET := UT_{{__TOP_MODULE_NAME__}}
VERBOSE := {{__VERBOSE__}}
EXAMPLE := {{__EXAMPLE__}}


all: release 

compile:
	cmake . -Bbuild -DSIMULATOR=$(SIMULATOR) -DTRACE=$(TRACE) -DPROJECT=$(PROJECT)
	cmake --build build --config Release --parallel `nproc`

release: compile
	@cp -r build/${TARGET} .
	@cp cpp/dut.cpp ${TARGET}/${TARGET}.cpp 
	@cp cpp/dut.hpp ${TARGET}/${TARGET}.hpp
	@cp dut_base.hpp ${TARGET}/dut_base.hpp

ifneq ($(EXAMPLE), OFF)
	@cp cpp/example.cpp ${TARGET}/example.cpp
	@cp cpp/cmake/${SIMULATOR}.cmake ${TARGET}/${PROJECT}.cmake
	@cp cpp/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp cpp/Makefile ${TARGET}/Makefile
	cd ${TARGET} && make
endif

ifneq ($(VERBOSE), OFF)
	cp *.v *.sv ${TARGET}/
else
# remove all files except UT_*
	rm -rf $(shell ls |grep -v "UT_")
endif

clean: 