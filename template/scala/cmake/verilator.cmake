add_definitions(-DUSE_VERILATOR)

function(XSScalaTarget)
	set(CMAKE_BUILD_RPATH $ORIGIN)

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
	set(JAR_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME})

	swig_add_library(UT_${PROJECT_NAME} LANGUAGE java SOURCES dut.i)
	set_target_properties(UT_${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${JAR_SOURCE_DIR})

	target_link_libraries(UT_${PROJECT_NAME} PRIVATE UT${RTLModuleName} xspcomm ${CustomLibs} ${CMAKE_DL_LIBS})
	target_link_options(UT_${PROJECT_NAME} PRIVATE 
		-Wl,-rpath=~/.local/lib
		-Wl,-rpath=/usr/local/lib 
		${CustomLinkOptions})

	set_property(TARGET UT_${PROJECT_NAME} PROPERTY SWIG_COMPILE_OPTIONS -package com.xspcomm)

	# copy file
	add_custom_command(
		OUTPUT ${JAR_SOURCE_DIR}/example.scala
		COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_CURRENT_BINARY_DIR}/*.java
				${JAR_SOURCE_DIR}/
		COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_CURRENT_SOURCE_DIR}/../scala/example.scala
				${JAR_SOURCE_DIR}/
		COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_CURRENT_SOURCE_DIR}/../scala/dut.java
				${JAR_SOURCE_DIR}/JavaUT_${PROJECT_NAME}.java
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
        DEPENDS ${JAR_SOURCE_DIR}/example.scala
	)

endfunction()
