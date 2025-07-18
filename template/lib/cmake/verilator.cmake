if(SIMULATOR STREQUAL "verilator")

	add_definitions(-DUSE_VERILATOR)

	# Get VERILATOR_ROOT from verilator command output
	execute_process(
		COMMAND bash -c "verilator -V|grep ROOT|grep verilator|head -n 1|awk '{print $3}'"
		OUTPUT_VARIABLE CMD_VERILATOR_ROOT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	find_package(verilator REQUIRED PATHS ${CMD_VERILATOR_ROOT} NO_DEFAULT_PATH)
	include_directories(${VERILATOR_ROOT}/include
											${VERILATOR_ROOT}/include/vltstd)

	# Trace
	if(${TRACE} STREQUAL "fst")
		set(TRACE_FLAG TRACE_FST)
		add_definitions(-DVL_TRACE)
	elseif(${TRACE} STREQUAL "vcd")
		set(TRACE_FLAG TRACE)
		add_definitions(-DVL_TRACE)
	else()
		set(TRACE_FLAG "")
	endif()

	# Readf filelist from file
	file(READ ${CMAKE_CURRENT_SOURCE_DIR}/filelist.f FILELIST)
	# set filelist to variable
	string(REGEX REPLACE "\n" ";" FILELIST "${FILELIST}")
	# set coverage flags
	if(${COVERAGE} STREQUAL "ON")
		set(COVERAGE_FLAG "COVERAGE")
		add_definitions(-DVL_COVERAGE)
	else()
		set(COVERAGE_FLAG "")
	endif()
	# set save/restore flags

	if(${CHECKPOINTS} STREQUAL "ON")
		add_definitions(-DVL_SAVEABLE)
		set(SIMULATOR_FLAGS ${SIMULATOR_FLAGS} "--savable")
	endif()
	# set vpi flags
	if(${VPI} STREQUAL "ON")
		add_definitions(-DVL_VPI)
		# add vpi flags in SIMULATOR_FLAGS
		set(SIMULATOR_FLAGS ${SIMULATOR_FLAGS} "--vpi" "--public-flat-rw")
	endif()

	set(SIMULATOR_FLAGS ${SIMULATOR_FLAGS} "--MAKEFLAGS" "-DVL_INLINE_OPT=inline" "-O3" "--instr-count-dpi" "0")
	message(STATUS "Verilator flags: ${SIMULATOR_FLAGS}")
	

	set(VERILATE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/DPI${ModuleName})
	include_directories(${VERILATE_DIRECTORY})

	add_library(DPI${ModuleName} STATIC)
	verilate(
		DPI${ModuleName}
		SOURCES
		${ModuleName}_top.sv
		${ModuleName}.v
		${FILELIST}
		TOP_MODULE
		${ModuleName}_top
		PREFIX
		V${ModuleName}
		INCLUDE_DIRS
		${CMAKE_CURRENT_SOURCE_DIR}
		DIRECTORY
		${VERILATE_DIRECTORY}
		${COVERAGE_FLAG}
		${TRACE_FLAG}
		OPT
		"-O3"
		VERILATOR_ARGS
		-Wno-fatal
		${SIMULATOR_FLAGS}
		-CFLAGS
		"-fPIC ${CFLAGS}")
	message(STATUS "Verilator generated files in ${VERILATE_DIRECTORY}")
	add_library(dut_base STATIC dut_base.cpp)
	add_dependencies(dut_base DPI${ModuleName})

	add_library(${ModuleName} SHARED dut_base)
	target_link_libraries(${ModuleName} "-Wl,--whole-archive" DPI${ModuleName}
	"-Wl,--no-whole-archive" dl )

	execute_process(
		COMMAND ${CMAKE_COMMAND} -E copy 
		${VERILATE_DIRECTORY}/V${ModuleName}__Dpi.h
		${CMAKE_BINARY_DIR}/UT_${ModuleName}/UT_${ModuleName}_dpi.hpp
	)

endif()
