if(SIMULATOR STREQUAL "verilator")

	add_definitions(-DUSE_VERILATOR)

	find_package(verilator REQUIRED)
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
	if( ${COVERAGE} STREQUAL "ON")
		set(COVERAGE_FLAG "COVERAGE")
		add_definitions(-DVL_COVERAGE)
	else()
		set(COVERAGE_FLAG "")
	endif()

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
		OPT_FAST
		VERILATOR_ARGS
		-Wno-fatal ${SIMULATOR_FLAGS}
		-CFLAGS "-fPIC ${CFLAGS}")
	message(STATUS "Verilator generated files in ${VERILATE_DIRECTORY}")
	add_library(dut_base STATIC dut_base.cpp)
	add_dependencies(dut_base DPI${ModuleName})

	add_library(${ModuleName} SHARED dut_base)
	target_link_libraries(${ModuleName} "-Wl,--whole-archive" DPI${ModuleName}
												"-Wl,--no-whole-archive")
	
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E copy ${VERILATE_DIRECTORY}/V${ModuleName}__Dpi.h ${CMAKE_BINARY_DIR}/UT_${ModuleName}/UT_${ModuleName}_dpi.hpp
	)

endif()
