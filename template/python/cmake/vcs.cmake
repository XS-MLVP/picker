add_definitions(-DUSE_VCS)

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
		"RTL_MODULE_NAME;"
		"CUSTOM_LIBS;CUSTOM_LINK_OPTIONS"
		""
		${ARGN}
	)
	add_definitions(-DUSE_VCS)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftls-model=global-dynamic")

	if (NOT DEFINED XSP_RTL_MODULE_NAME)
		message(FATAL_ERROR "RTL_MODULE_NAME not defined")
	endif()


	set(RTLModuleName ${XSP_RTL_MODULE_NAME})
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
	include_directories(${VCS_HOME}/include ${VCS_HOME}/linux64/lib/)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(UT${RTLModuleName} SHARED IMPORTED)
	set_property(
		TARGET UT${RTLModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_SOURCE_DIR}/libUT${RTLModuleName}.so)

	# Workaround for VCS
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

	set_property(SOURCE dut.i PROPERTY CPLUSPLUS ON)
	swig_add_library(UT_${PROJECT_NAME} LANGUAGE python SOURCES dut.i)

	target_link_libraries(UT_${PROJECT_NAME} PRIVATE UT${RTLModuleName} DPI${RTLModuleName} xspcomm ${CustomLibs} ${CMAKE_DL_LIBS})
	search_libs(ALL_FOUND LIBS zerosoft_rt_stubs m c pthread numa dl)
	target_link_options(
		UT_${PROJECT_NAME}
		PRIVATE
		-L./
		-L${VCS_HOME}/linux64/lib
		-Wl,-rpath=~/.local/lib
		-Wl,-rpath=/usr/local/lib
		-Wl,-rpath=${VCS_HOME}/linux64/lib
		-no-pie
		-Wl,--no-as-needed
		-rdynamic
		-Wl,-whole-archive
		-lvcsucli
		-Wl,-no-whole-archive
		-luclinative
		-lvirsim
		-lerrorinf
		-lsnpsmalloc
		-lvfs
		-lvcsnew
		-lsimprofile
		${LIBS}
		${CustomLinkOptions})

endfunction()
