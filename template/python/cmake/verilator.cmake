add_definitions(-DUSE_VERILATOR)

function(XSPyTarget)

	cmake_parse_arguments(
		XSP
		""
		"RTL_MODULE_NAME;"
		"CUSTOM_LIBS;CUSTOM_LINK_OPTIONS"
		""
		${ARGN}
	)

	if (NOT DEFINED XSP_RTL_MODULE_NAME)
		message(FATAL_ERROR "RTL_MODULE_NAME not defined")
	endif()

	set(RTLModuleName ${XSP_RTL_MODULE_NAME})
	set(CustomLibs ${XSP_CUSTOM_LIBS})
	set(CustomLinkOptions ${XSP_CUSTOM_LINK_OPTIONS})

	find_package(verilator REQUIRED)
	include_directories(${VERILATOR_ROOT}/include
											${VERILATOR_ROOT}/include/vltstd)


	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(UT${RTLModuleName} SHARED IMPORTED)
	set_property(
		TARGET UT${RTLModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_SOURCE_DIR}/libUT${RTLModuleName}.so)

	set_property(SOURCE dut.i PROPERTY CPLUSPLUS ON)
	swig_add_library(UT_${PROJECT_NAME} LANGUAGE python SOURCES dut.i)

	target_link_libraries(UT_${PROJECT_NAME} PRIVATE UT${RTLModuleName} xspcomm ${CustomLibs} ${CMAKE_DL_LIBS})
	target_link_options(UT_${PROJECT_NAME} PRIVATE 
		-Wl,-rpath=./ 
		-Wl,-rpath=~/.local/lib
		-Wl,-rpath=/usr/local/lib 
		${CustomLinkOptions})

endfunction()
