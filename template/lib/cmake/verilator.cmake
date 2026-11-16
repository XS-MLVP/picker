cmake_policy(SET CMP0058 NEW)
if(SIMULATOR STREQUAL "verilator")

	add_definitions(-DUSE_VERILATOR)

	# Get VERILATOR_ROOT from verilator command output
	execute_process(
		COMMAND bash -c "verilator -V|grep ROOT|grep verilator|tail -n 1|awk '{print $3}'"
		OUTPUT_VARIABLE CMD_VERILATOR_ROOT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	find_package(verilator 5.020 REQUIRED PATHS ${CMD_VERILATOR_ROOT} NO_DEFAULT_PATH)
{% if __HAS_AGGREGATE_VIEWS__ %}
	if(verilator_VERSION VERSION_LESS "5.048")
		message(FATAL_ERROR "Struct and multidimensional packed-array views require Verilator 5.048 or newer (found ${verilator_VERSION})")
	endif()
{% endif %}
	include_directories(${VERILATOR_ROOT}/include
											${VERILATOR_ROOT}/include/vltstd)

	# Trace
	if("${TRACE}" STREQUAL "fst")
		set(TRACE_FLAG TRACE_FST)
		add_definitions(-DVL_TRACE)
	elseif("${TRACE}" STREQUAL "vcd")
		if(verilator_VERSION VERSION_LESS "5.036")
			set(TRACE_FLAG TRACE)
		else()
			set(TRACE_FLAG TRACE_VCD)
		endif()
		add_definitions(-DVL_TRACE)
	else()
		set(TRACE_FLAG "")
	endif()
	message(STATUS "Verilator ${verilator_VERSION}, trace option: ${TRACE_FLAG}")

	# Filelist is passed to verilator with -f below, it reports the files it reads
	# back to cmake through <prefix>_DEPS
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

	set(SIMULATOR_FLAGS ${SIMULATOR_FLAGS} "--MMD" "--MP" "--MAKEFLAGS" "-DVL_INLINE_OPT=inline" "-O3" "--instr-count-dpi" "0")
	message(STATUS "Verilator flags: ${SIMULATOR_FLAGS}")


	set(VERILATE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/DPI${ModuleName})
	include_directories(${VERILATE_DIRECTORY})

	if("${CMAKE_CURRENT_SOURCE_DIR}" MATCHES "[ \t]")
		# verilator-config.cmake writes its generated source lists with spaces
		# as separators, so a project path containing a space becomes several
		# bogus source files at configure time. For spaced paths, run Verilator
		# directly and consume the manifest, which keeps every path a distinct
		# CMake list item. Normal paths keep using the upstream helper.
		set(PICKER_VERILATOR_ARGS
			--cc
			--top ${ModuleName}_top
			-y ${CMAKE_CURRENT_SOURCE_DIR}
			-Wno-fatal
			-f ${CMAKE_CURRENT_SOURCE_DIR}/filelist.f
			${SIMULATOR_FLAGS})
		if("${COVERAGE}" STREQUAL "ON")
			list(APPEND PICKER_VERILATOR_ARGS --coverage)
		endif()
		if("${TRACE}" STREQUAL "fst")
			list(APPEND PICKER_VERILATOR_ARGS --trace-fst)
		elseif("${TRACE}" STREQUAL "vcd")
			# --trace-vcd only exists from 5.036; older releases spell the
			# same option --trace. TRACE_FLAG above already encodes this.
			if(verilator_VERSION VERSION_LESS "5.036")
				list(APPEND PICKER_VERILATOR_ARGS --trace)
			else()
				list(APPEND PICKER_VERILATOR_ARGS --trace-vcd)
			endif()
		endif()
		string(TOLOWER "${CMAKE_CXX_COMPILER_ID}" PICKER_VERILATOR_COMPILER)
		if(PICKER_VERILATOR_COMPILER STREQUAL "appleclang")
			set(PICKER_VERILATOR_COMPILER clang)
		elseif(NOT PICKER_VERILATOR_COMPILER MATCHES "^msvc$|^clang$")
			set(PICKER_VERILATOR_COMPILER gcc)
		endif()
		execute_process(
			COMMAND ${CMAKE_COMMAND} -E env VERILATOR_ROOT=${VERILATOR_ROOT}
				${VERILATOR_BIN}
				--compiler ${PICKER_VERILATOR_COMPILER}
				--prefix V${ModuleName}
				--Mdir ${VERILATE_DIRECTORY}
				--make json
				${PICKER_VERILATOR_ARGS}
				${ModuleName}_top.sv
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			RESULT_VARIABLE PICKER_VERILATOR_STATUS
			OUTPUT_VARIABLE PICKER_VERILATOR_OUTPUT
			ERROR_VARIABLE PICKER_VERILATOR_OUTPUT)
		if(PICKER_VERILATOR_STATUS)
			message(FATAL_ERROR "Verilator command failed (return code=${PICKER_VERILATOR_STATUS})\n${PICKER_VERILATOR_OUTPUT}")
		endif()
		file(READ ${VERILATE_DIRECTORY}/V${ModuleName}.json PICKER_VERILATOR_MANIFEST)
		set(PICKER_VERILATED_SOURCES "")
		foreach(PICKER_GROUP global classes_slow classes_fast support_slow support_fast user_classes)
			string(JSON PICKER_COUNT ERROR_VARIABLE PICKER_JSON_ERROR LENGTH
				"${PICKER_VERILATOR_MANIFEST}" sources ${PICKER_GROUP})
			if(PICKER_JSON_ERROR OR NOT PICKER_COUNT)
				continue()
			endif()
			math(EXPR PICKER_LAST "${PICKER_COUNT}-1")
			foreach(PICKER_INDEX RANGE ${PICKER_LAST})
				string(JSON PICKER_SOURCE GET "${PICKER_VERILATOR_MANIFEST}" sources ${PICKER_GROUP} ${PICKER_INDEX})
				list(APPEND PICKER_VERILATED_SOURCES "${PICKER_SOURCE}")
			endforeach()
		endforeach()
		add_library(DPI${ModuleName} STATIC ${PICKER_VERILATED_SOURCES})
		# The static library is always linked into the shared DUT library, so
		# every object must be position independent.
		set_target_properties(DPI${ModuleName} PROPERTIES POSITION_INDEPENDENT_CODE ON)
		target_include_directories(DPI${ModuleName} PUBLIC
			${VERILATOR_ROOT}/include ${VERILATOR_ROOT}/include/vltstd ${VERILATE_DIRECTORY})
		if("${COVERAGE}" STREQUAL "ON")
			target_compile_definitions(DPI${ModuleName} PRIVATE VM_COVERAGE=1)
		else()
			target_compile_definitions(DPI${ModuleName} PRIVATE VM_COVERAGE=0)
		endif()
		# The upstream helper forwards --cflag through verilator's own
		# makefile; here CMake compiles the sources, so forward it here.
		if(NOT "${CFLAGS}" STREQUAL "")
			separate_arguments(PICKER_CFLAGS NATIVE_COMMAND "${CFLAGS}")
			target_compile_options(DPI${ModuleName} PRIVATE ${PICKER_CFLAGS})
		endif()
		if("${TRACE}" STREQUAL "fst")
			target_compile_definitions(DPI${ModuleName} PRIVATE VM_TRACE=1 VM_TRACE_FST=1)
			target_link_libraries(DPI${ModuleName} PUBLIC -llz4 -lz)
		elseif("${TRACE}" STREQUAL "vcd")
			target_compile_definitions(DPI${ModuleName} PRIVATE VM_TRACE=1 VM_TRACE_VCD=1)
		else()
			target_compile_definitions(DPI${ModuleName} PRIVATE VM_TRACE=0)
		endif()
		target_link_libraries(DPI${ModuleName} PUBLIC ${VERILATOR_MT_CFLAGS})
	else()
		add_library(DPI${ModuleName} STATIC)
		verilate(
			DPI${ModuleName}
			SOURCES
			${ModuleName}_top.sv
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
			-f
			${CMAKE_CURRENT_SOURCE_DIR}/filelist.f
			${SIMULATOR_FLAGS}
			-CFLAGS
			"-fPIC ${CFLAGS}")
	endif()
	message(STATUS "Verilator generated files in ${VERILATE_DIRECTORY}")
	add_library(dut_base STATIC dut_base.cpp)
	add_dependencies(dut_base DPI${ModuleName})

	add_library(${ModuleName} SHARED dut_base)

	if (APPLE)
		target_link_libraries(${ModuleName} PRIVATE
			"-Wl,-force_load,$<TARGET_FILE:DPI${ModuleName}>"
			dl
			z
    	)
	else()
    	target_link_libraries(${ModuleName} PRIVATE
			"-Wl,--whole-archive"
			DPI${ModuleName}
			"-Wl,--no-whole-archive"
			dl
    	)
	endif()

	execute_process(
		COMMAND ${CMAKE_COMMAND} -E copy
		${VERILATE_DIRECTORY}/V${ModuleName}__Dpi.h
		${CMAKE_BINARY_DIR}/UT_${ModuleName}/UT_${ModuleName}_dpi.hpp
	)

endif()
