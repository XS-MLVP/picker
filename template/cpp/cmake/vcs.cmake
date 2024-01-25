add_definitions(-DUSE_VCS)

function(XSPTarget)

	cmake_parse_arguments(
		XSP
		""
		"RTL_MODULE_NAME;EXECUTABLE_NAME"
		"CUSTOM_LIBS;CUSTOM_LINK_OPTIONS"
		""
		${ARGN}
	)
	add_definitions(-DUSE_VCS)

	if (NOT DEFINED XSP_RTL_MODULE_NAME)
		message(FATAL_ERROR "RTL_MODULE_NAME not defined")
	endif()

	if (NOT DEFINED XSP_EXECUTABLE_NAME)
		message(FATAL_ERROR "EXECUTABLE_NAME not defined")
	endif()

	set(RTLModuleName ${XSP_RTL_MODULE_NAME})
	set(ExecutableName ${XSP_EXECUTABLE_NAME})
	set(CustomLibs ${XSP_CUSTOM_LIBS})
	set(CustomLinkOptions ${XSP_CUSTOM_LINK_OPTIONS})

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

	# Build the cpp wrapper
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(UT${RTLModuleName} SHARED IMPORTED)
	set_property(
		TARGET UT${RTLModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_SOURCE_DIR}/libUT${RTLModuleName}.so)

	# Workaround for VCS
	include_directories(${VCS_HOME}/include ${VCS_HOME}/linux64/lib)
	execute_process(
		COMMAND
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/libDPI${RTLModuleName}.so
			${CMAKE_CURRENT_BINARY_DIR}/libDPI${RTLModuleName}.so)
	add_library(DPI${RTLModuleName} SHARED IMPORTED)
	set_property(
		TARGET DPI${RTLModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_BINARY_DIR}/libDPI${RTLModuleName}.so)

	# Build the test executable
	add_executable(${ExecutableName} UT_${RTLModuleName}.cpp)
	target_link_libraries(${ExecutableName} UT${RTLModuleName} DPI${RTLModuleName} xspcomm ${CustomLibs} ${CMAKE_DL_LIBS})
	target_link_options(
		${ExecutableName}
		PRIVATE
		-L./
		-L${VCS_HOME}/linux64/lib
		-Wl,-rpath=./ 
		-Wl,-rpath=~/.local/lib
		-Wl,-rpath=/usr/local/lib 
		-Wl,-rpath=${VCS_HOME}/linux64/lib
		-no-pie
		-Wl,--no-as-needed
		-rdynamic
		-Wl,-whole-archive
		-lvcsucli
		-Wl,-no-whole-archive
		-lzerosoft_rt_stubs
		-luclinative
		-lvirsim
		-lerrorinf
		-lsnpsmalloc
		-lvfs
		-lvcsnew
		-lsimprofile
		-ldl
		-lc
		-lm
		-lpthread
		-lnuma
		${CustomLinkOptions})

endfunction()
