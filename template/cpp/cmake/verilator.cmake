add_definitions(-DUSE_VERILATOR)

function(XSPTarget)

	cmake_parse_arguments(
		XSP
		""
		"RTL_MODULE_NAME;EXECUTABLE_NAME"
		"CUSTOM_LIBS;CUSTOM_LINK_OPTIONS"
		""
		${ARGN}
	)
	add_definitions(-DUSE_VERILATOR)

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

	execute_process(
		COMMAND bash -c "verilator -V|grep ROOT|grep verilator|head -n 1|awk '{print $3}'"
		OUTPUT_VARIABLE CMD_VERILATOR_ROOT
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	find_package(verilator REQUIRED PATHS ${CMD_VERILATOR_ROOT}  NO_DEFAULT_PATH)
	include_directories(${VERILATOR_ROOT}/include
											${VERILATOR_ROOT}/include/vltstd)


	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(UT${RTLModuleName} SHARED IMPORTED)
	set_property(
		TARGET UT${RTLModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_SOURCE_DIR}/libUT${RTLModuleName}.so)
	add_executable(${ExecutableName} UT_${RTLModuleName}.cpp)
	target_link_libraries(${ExecutableName} UT${RTLModuleName} xspcomm ${CustomLibs} ${CMAKE_DL_LIBS})
	target_link_options(${ExecutableName} PRIVATE 
		-Wl,-rpath={{__XSPCOMM_LIB__}}
		-Wl,-rpath=~/.local/lib
		-Wl,-rpath=/usr/local/lib  
		${CustomLinkOptions})

endfunction()
