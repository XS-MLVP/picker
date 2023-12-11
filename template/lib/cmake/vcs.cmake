if(SIMULATOR STREQUAL "vcs")

	add_definitions(-DUSE_VCS)

	# Find VCS
	if(DEFINED ENV{VCS_HOME})
		set(VCS_HOME $ENV{VCS_HOME})
		message(STATUS "Find VCS root: ${VCS_HOME}")
	else()
		message(FATAL_ERROR "Cannot find vcs, please install (vcs)")
	endif()

	# Find Verdi
	if(DEFINED ENV{VERDI_HOME})
		set(VERDI_HOME $ENV{VERDI_HOME})
		message(STATUS "Find Verdi root: ${VERDI_HOME}")
	else()
		message(FATAL_ERROR "Cannot find verdi, please install (verdi)")
	endif()

	# Add VCS include path
	include_directories(${VCS_HOME}/include ${VCS_HOME}/linux64/lib)

	# Add VCS magic
	set(CMAKE_EXE_LINKER_FLAGS
			"${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=1048576")

	# Copy all source files to build directory
	file(GLOB_RECURSE SOURCES "*.sv" "*.v")
	file(COPY ${SOURCES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

	# VCS compile
	execute_process(
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND
			vcs -e VcsMain -slave ${VCS_TRACE} -sverilog -lca -l compile.log -full64
			-timescale=1ns/1ps ${ModuleName}_top.sv ${ModuleName}.v -o
			libDPI${ModuleName}.so +modelsave -LDFLAGS "-shared" ${SIMULATOR_FLAGS}
			-P ${VERDI_HOME}/share/PLI/VCS/LINUX64/novas.tab
			${VERDI_HOME}/share/PLI/VCS/LINUX64/pli.a)

	link_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(DPI${ModuleName} SHARED IMPORTED)
	set_target_properties(DPI${ModuleName} PROPERTIES IMPORTED_LOCATION
																										libDPI${ModuleName}.so)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
	add_library(${ModuleName} SHARED dut_base)
	target_link_libraries(${ModuleName} PRIVATE DPI${ModuleName})
	target_link_options(${ModuleName} PRIVATE -Wl,-rpath,./)

	# Copy libDPI${ModuleName}.so.daidir directory to build directory
	add_custom_command(
		TARGET ${ModuleName}
		POST_BUILD
		COMMAND
			${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_BINARY_DIR}/libDPI${ModuleName}.so.daidir
			${CMAKE_BINARY_DIR}/UT_${ModuleName}/libDPI${ModuleName}.so.daidir
		COMMAND 
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_BINARY_DIR}/vc_hdrs.h
			${CMAKE_BINARY_DIR}/UT_${ModuleName}/UT_${ModuleName}_dpi.hpp)

endif()
