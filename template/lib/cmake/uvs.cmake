if(SIMULATOR STREQUAL "uvs")

	add_definitions(-DUSE_UVS)

	# Find UVS
	if(DEFINED ENV{UVS_HOME})
		set(UVS_HOME $ENV{UVS_HOME})
		message(STATUS "Find UVS root: ${UVS_HOME}")
	else()
		message(FATAL_ERROR "Cannot find ENV{UVS_HOME}, please install (uvs)")
	endif()

	# Find UVD
	if(DEFINED ENV{UVD_HOME})
		set(VERDI_HOME $ENV{UVD_HOME})
		message(STATUS "Find UVD root: ${UVD_HOME}")
	else()
		message(FATAL_ERROR "Cannot find ENV{UVD_HOME}, please install (uvd) or set ENV{UVD_HOME}")
	endif()

	# Add UVS include path
	include_directories(${UVS_HOME}/include)

	# Add VCS magic
	set(CMAKE_EXE_LINKER_FLAGS
			"${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=1048576")

	# Copy all source files to build directory
	file(GLOB_RECURSE SOURCES "*.sv" "*.v" "*.f")
	file(COPY ${SOURCES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

	# CFLAGS Detect IF CFLAGS is not empty, add -cflags to VCS compile
	set(SIMULATOR_CFLAGS "")
	if(NOT "${CFLAGS}" STREQUAL "")
		# add -CFLAGS to VCS compile
		set(SIMULATOR_CFLAGS "-cflags;${CFLAGS}")
		message(STATUS "UVS CFLAGS: ${CFLAGS}")
	endif()
	
	# if vpi is enabled, add SIMULATOR_FLAGS '+vpi' and '-debug_access+all'
	if(${VPI} STREQUAL "ON")
		set(SIMULATOR_FLAGS "${SIMULATOR_FLAGS};-debug cbk")
	endif()


	# Using VCS compile
		execute_process(
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND
                uvs -ddb -debug r -sv -Xair opt.jemalloc=0 -Xair slave_mode -Xair sim_entry=UvsMain -l uvs_compile.log
                -top ${ModuleName}_top -timescale=1ns/1ps    
                ${ModuleName}_top.sv ${ModuleName}.v -f filelist.f    
                -o libDPI${ModuleName}.so                              
                ${SIMULATOR_FLAGS}                                                                 
                -cflags ${SIMULATOR_CFLAGS})

	# Set VCS DPI library as imported library
	link_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(DPI${ModuleName} SHARED IMPORTED)
	set_target_properties(DPI${ModuleName} PROPERTIES IMPORTED_LOCATION libDPI${ModuleName}.so)

	# Compile libUT wrapper with VCS DPI library
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
			${CMAKE_CURRENT_BINARY_DIR}/libDPI${ModuleName}.so.db
			${CMAKE_BINARY_DIR}/UT_${ModuleName}/libDPI${ModuleName}.so.db
		COMMAND touch ${CMAKE_BINARY_DIR}/UT_${ModuleName}/UT_${ModuleName}_dpi.hpp)

endif()
