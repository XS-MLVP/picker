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
export RW_TYPE := {{__RW_TYPE__}}
NPROC := $(shell (nproc 2>/dev/null || sysctl -n hw.ncpu) 2>/dev/null)

ifeq ($(SIMULATOR), verilator)
MEM_DIRECT_TARGET_FILE := build/DPI{{__TOP_MODULE_NAME__}}/V{{__TOP_MODULE_NAME__}}___024root.h
endif
ifeq ($(SIMULATOR), gsim)
MEM_DIRECT_TARGET_FILE := build/_SRC/{{__TOP_MODULE_NAME__}}.h
endif

all: release 

compile:
	mkdir -p build
	cp pli.tab build/pli.tab
	@echo -n "[TIME LOG] {{__SIMULATOR__}} generate START: "  && date +%s.%3N
	/usr/bin/time cmake . -Bbuild -DSIMULATOR=$(SIMULATOR) -DTRACE=$(TRACE) -DPROJECT=$(PROJECT) -DCMAKE_BUILD_PARALLEL=$(NPROC) -DRW_TYPE=$(RW_TYPE)
	@echo -n "[TIME LOG] {{__SIMULATOR__}} generate END: " && date +%s.%3N
	/usr/bin/time cmake --build build --config release --parallel $(NPROC)
	@echo -n "[TIME LOG] {{__SIMULATOR__}} build END: " && date +%s.%3N

ifeq ($(RW_TYPE), MEM_DIRECT)
ifdef MEM_DIRECT_TARGET_FILE
	@echo "MEM_DIRECT_TARGET_FILE: ${MEM_DIRECT_TARGET_FILE}"
# file exsit check with Makefile
ifeq ($(wildcard ${MEM_DIRECT_TARGET_FILE}),)
	@echo -n "[TIME LOG] mem_direct export START: " && date +%s.%3N
	@rm -rf mem_direct
	@{{__GENERATOR_PICKER_PATH__}} export ${MEM_DIRECT_TARGET_FILE} \
		--source_dir {{__GENERATOR_TEMPLATE_PATH__}}/mem_direct --target_dir ./mem_direct \
		--source_module_name {{__TOP_MODULE_NAME__}} --sim {{__SIMULATOR__}}
	@echo -n "[TIME LOG] mem_direct export END: " && date +%s.%3N
endif
	@echo -n "[TIME LOG] mem_direct gen START: " && date +%s.%3N
	@cd mem_direct && make -j$(NPROC) TOP_NAME={{__TOP_MODULE_NAME__}}
	@./mem_direct/export.bin > mem_direct/{{__TOP_MODULE_NAME__}}_offset.yaml
	@rm -rf mem_direct_tmp
	@echo -n "[TIME LOG] mem_direct gen END: " && date +%s.%3N
endif
endif

release: compile
	@cp -r build/${TARGET} .
	@cp build/_SRC/*.h ${TARGET}/ 2>/dev/null|| true
	@cp build/_SRC/*.hpp ${TARGET}/ 2>/dev/null|| true
	@cp *.hpp ${TARGET}/
	@cp mem_direct/{{__TOP_MODULE_NAME__}}_offset.yaml ${TARGET}/{{__TOP_MODULE_NAME__}}_offset.yaml 2>/dev/null|| true
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
