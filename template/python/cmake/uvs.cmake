add_definitions(-DUSE_UVS)


function(search_libs afound rlibs)
    set(libs_list ${ARGN})
    set(all_found TRUE)
    set(libs "")
    foreach(lib ${libs_list})
        find_library(LIB_${lib} ${lib})
        if (NOT LIB_${lib})
            set(all_found FALSE)
            break()
        else()
            message(STATUS "Found ${lib}: ${LIB_${lib}}")
            set(libs "${libs} -l${lib}")
        endif()
    endforeach()
    set(${afound} ${all_found} PARENT_SCOPE)
    set(${rlibs} ${libs} PARENT_SCOPE)
endfunction()


function(XSPyTarget)

	cmake_parse_arguments(
		XSP
		""
		"RTL_MODULE_NAME"
		"CUSTOM_LIBS;CUSTOM_LINK_OPTIONS"
		""
		${ARGN}
	)
	add_definitions(-DUSE_UVS)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftls-model=global-dynamic")

	if (NOT DEFINED XSP_RTL_MODULE_NAME)
		message(FATAL_ERROR "RTL_MODULE_NAME not defined")
	endif()

	set(RTLModuleName ${XSP_RTL_MODULE_NAME})
	set(CustomLibs ${XSP_CUSTOM_LIBS})
	set(CustomLinkOptions ${XSP_CUSTOM_LINK_OPTIONS})

	# Find UVS
	if(DEFINED ENV{UVS_HOME})
		set(UVS_HOME $ENV{UVS_HOME})
		message(STATUS "Find UVS root: ${UVS_HOME}")
	else()
		message(FATAL_ERROR "Cannot find uvs, please install (uvs)")
	endif()

	# Find UVD
	if(DEFINED ENV{UVD_HOME})
		set(UVD_HOME $ENV{UVD_HOME})
		message(STATUS "Find UVD root: ${UVD_HOME}")
	else()
		message(FATAL_ERROR "Cannot find uvd, please install (uvd)")
	endif()

	# Build the cpp wrapper
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(UT${RTLModuleName} SHARED IMPORTED)
	set_property(
		TARGET UT${RTLModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_SOURCE_DIR}/libUT${RTLModuleName}.so)

	include_directories(${UVS_HOME}/include)
	execute_process(
		COMMAND
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/libDPI${RTLModuleName}.so
			${CMAKE_CURRENT_BINARY_DIR}/libDPI${RTLModuleName}.so
		COMMAND
			${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_SOURCE_DIR}/libDPI${RTLModuleName}.so.db
			${CMAKE_CURRENT_BINARY_DIR}/libDPI${RTLModuleName}.so.db)

	add_library(DPI${RTLModuleName} SHARED IMPORTED)
	set_property(
		TARGET DPI${RTLModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_BINARY_DIR}/libDPI${RTLModuleName}.so)

	# Build the test executable
	set_property(SOURCE dut.i PROPERTY CPLUSPLUS ON)
	swig_add_library(UT_${PROJECT_NAME} LANGUAGE python SOURCES dut.i)
	target_link_libraries(UT_${PROJECT_NAME} PRIVATE UT${RTLModuleName} DPI${RTLModuleName} xspcomm
	                      ${CustomLibs} ${CMAKE_DL_LIBS} Python3::Module)

	search_libs(ALL_FOUND LIBS zerosoft_rt_stubs m c pthread numa dl)
	# -Wl,--unresolved-symbols=ignore-in-shared-libs
	target_link_options(
		UT_${PROJECT_NAME}
		PRIVATE
		-L./		
		-Wl,-rpath={{__XSPCOMM_LIB__}}
		-Wl,-rpath=~/.local/lib
		-Wl,-rpath=/usr/local/lib
		-Wl,--as-need
		${LIBS}
		${CustomLinkOptions})

endfunction()
