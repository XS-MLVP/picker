add_definitions(-DUSE_VCS)

function(XSScalaTarget)

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
	set(JAR_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})

	swig_add_library(UT_${PROJECT_NAME} LANGUAGE java SOURCES dut.i)
	set_target_properties(UT_${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${JAR_SOURCE_DIR})

	target_link_libraries(UT_${PROJECT_NAME} PRIVATE UT${RTLModuleName} DPI${RTLModuleName} xspcomm ${CustomLibs} ${CMAKE_DL_LIBS})
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

	set_property(TARGET UT_${PROJECT_NAME} PROPERTY SWIG_COMPILE_OPTIONS -package com.xspcomm)

	# copy file
	add_custom_command(
		OUTPUT ${JAR_SOURCE_DIR}/simple_step.scala
		COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_CURRENT_BINARY_DIR}/*.java
				${JAR_SOURCE_DIR}/
		COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_CURRENT_SOURCE_DIR}/../scala/simple_step.scala
				${JAR_SOURCE_DIR}/
		COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_CURRENT_SOURCE_DIR}/../scala/dut.java
				${JAR_SOURCE_DIR}/UT_${PROJECT_NAME}.java
		COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_CURRENT_SOURCE_DIR}/../scala/dut.scala
				${JAR_SOURCE_DIR}/UT_${PROJECT_NAME}.scala
		COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_CURRENT_SOURCE_DIR}/*.so
				${JAR_SOURCE_DIR}/
		COMMAND ${Java_JAVAC_EXECUTABLE} -d ${JAR_SOURCE_DIR} ${JAR_SOURCE_DIR}/*.java -cp ${CMAKE_CURRENT_SOURCE_DIR}/xspcomm-scala.jar
		COMMAND scalac -cp ${CMAKE_CURRENT_SOURCE_DIR}/xspcomm-scala.jar -d ${JAR_SOURCE_DIR} -classpath ${JAR_SOURCE_DIR} ${JAR_SOURCE_DIR}/*.scala
		DEPENDS UT_${PROJECT_NAME}
	)
	add_custom_target(
    _DummyTarget_create_${PROJECT_NAME} ALL
        COMMAND ${Java_JAR_EXECUTABLE} cfm UT_${PROJECT_NAME}-scala.jar MANIFEST.MF -C ${JAR_SOURCE_DIR} .
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${JAR_SOURCE_DIR}/simple_step.scala
	)

endfunction()
