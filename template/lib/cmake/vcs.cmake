if(SIMULATOR STREQUAL "vcs")

	add_definitions(-DUSE_VCS)

	# Find VCS
	if(DEFINED ENV{VCS_HOME})
		set(VCS_HOME $ENV{VCS_HOME})
		message(STATUS "Find VCS root: ${VCS_HOME}")
	else()
		message(FATAL_ERROR "Cannot find ENV{VCS_HOME}, please install (vcs)")
	endif()

	# Find Verdi
	if(DEFINED ENV{VERDI_HOME})
		set(VERDI_HOME $ENV{VERDI_HOME})
		message(STATUS "Find Verdi root: ${VERDI_HOME}")
	else()
		message(FATAL_ERROR "Cannot find ENV{VERDI_HOME}, please install (verdi) or set ENV{VERDI_HOME}")
	endif()

	# Add VCS include path
	include_directories(${VCS_HOME}/include ${VCS_HOME}/linux64/lib)

	# Add VCS magic
	set(CMAKE_EXE_LINKER_FLAGS
			"${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=1048576")

	# Copy all source files to build directory
	file(GLOB_RECURSE SOURCES "*.sv" "*.v" "*.f")
	file(COPY ${SOURCES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

	# CFLAGS Detect IF CFLAGS is not empty, add -cflags to VCS compile
	separate_arguments(CFLAGS)
	if(NOT "${CFLAGS}" STREQUAL "")
		foreach(CFLAG ${CFLAGS})
			set(SIMULATOR_FLAGS "${SIMULATOR_FLAGS} -cflags '${CFLAG}'")
		endforeach()
		message(STATUS "VCS CFLAGS: ${SIMULATOR_FLAGS}")
	endif()

	# Using Compiled vcs dynamic library
	if(NOT "${VCS_DYN}" STREQUAL "")
		message(STATUS "Using VCS dyn target: ${VCS_DYN}")
		file(COPY ${VCS_DYN} ${VCS_DYN}.daidir DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
		# Extract Include path has been added by ${EXTRA_INC_LIST} and check if it is empty
		if("${EXTRA_INC_LIST}" STREQUAL "")
			message(WARNING "EXTRA_INC_LIST is empty, please check your CMakeLists.txt")
		endif()
	else()
	# Using VCS compile
		execute_process(
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND
				vcs -e VcsMain -slave ${VCS_TRACE} -sverilog -lca -l compile.log
				-full64 -timescale=1ns/1ps ${ModuleName}_top.sv ${ModuleName}.v -f
				filelist.f -o libDPI${ModuleName}.so +modelsave -LDFLAGS "-shared"
				${SIMULATOR_FLAGS} -P ${VERDI_HOME}/share/PLI/VCS/LINUX64/novas.tab
				${VERDI_HOME}/share/PLI/VCS/LINUX64/pli.a)
	endif()

	# Add VCS dependency library
	add_library(vcs_tls OBJECT IMPORTED)
	set_target_properties(vcs_tls PROPERTIES IMPORTED_OBJECTS ${VCS_HOME}/linux64/lib/vcs_tls.o)
	add_library(vcs_save_restore OBJECT IMPORTED)
	set_target_properties(
		vcs_save_restore PROPERTIES IMPORTED_OBJECTS ${VCS_HOME}/linux64/lib/vcs_save_restore.o)

	# Set VCS DPI library as imported library
	link_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(DPI${ModuleName} SHARED IMPORTED)
	set_target_properties(DPI${ModuleName} PROPERTIES IMPORTED_LOCATION libDPI${ModuleName}.so)

	# Compile libUT wrapper with VCS DPI library
	include_directories(${CMAKE_CURRENT_BINARY_DIR})
	add_library(${ModuleName} SHARED dut_base)
	target_link_libraries(${ModuleName} PRIVATE DPI${ModuleName} vcs_tls vcs_save_restore)
	target_link_options(${ModuleName} PRIVATE -Wl,-rpath,./)

	# Copy libDPI${ModuleName}.so.daidir directory to build directory
	add_custom_command(
		TARGET ${ModuleName}
		POST_BUILD
		COMMAND
			${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_BINARY_DIR}/libDPI${ModuleName}.so.daidir
			${CMAKE_BINARY_DIR}/UT_${ModuleName}/libDPI${ModuleName}.so.daidir
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/vc_hdrs.h
						${CMAKE_BINARY_DIR}/UT_${ModuleName}/UT_${ModuleName}_dpi.hpp)

endif()
