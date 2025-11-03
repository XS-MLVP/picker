add_definitions(-DUSE_VERILATOR)

function(XSPyTarget)
	set(CMAKE_BUILD_RPATH "")
	if(CMAKE_SYSTEM_NAME MATCHES "Darwin") # macOS
		list(APPEND CMAKE_BUILD_RPATH "@loader_path")
	elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
		list(APPEND CMAKE_BUILD_RPATH "$ORIGIN")
	endif()

	list(APPEND CMAKE_BUILD_RPATH "{{__XSPCOMM_LIB__}}")
	list(APPEND CMAKE_BUILD_RPATH "$ENV{HOME}/.local/lib")
	list(APPEND CMAKE_BUILD_RPATH "/usr/local/lib")

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

	execute_process(
		COMMAND bash -c "verilator -V|grep ROOT|grep verilator|tail -n 1|awk '{print $3}'"
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
						 ${CMAKE_CURRENT_SOURCE_DIR}/libUT${RTLModuleName}${CMAKE_SHARED_LIBRARY_SUFFIX})

	set_property(SOURCE dut.i PROPERTY CPLUSPLUS ON)
	swig_add_library(UT_${PROJECT_NAME} LANGUAGE python SOURCES dut.i)

	target_link_libraries(UT_${PROJECT_NAME} PRIVATE UT${RTLModuleName} xspcomm ${CustomLibs} ${CMAKE_DL_LIBS} Python3::Module)
	target_link_options(UT_${PROJECT_NAME} PRIVATE ${CustomLinkOptions})


endfunction()
