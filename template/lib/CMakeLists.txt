cmake_minimum_required(VERSION 3.11)

set(ModuleName ${PROJECT})
project(${ModuleName} VERSION 1.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# enable coroutines
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines -ftls-model=global-dynamic $ENV{CFLAGS}")
set(SIMULATOR_FLAGS "$ENV{SIMULATOR_FLAGS}")
set(CFLAGS "$ENV{CFLAGS}")
set(COVERAGE "$ENV{COVERAGE}")

# Non-root install
set(NON_ROOT_INC_DIR "$ENV{HOME}/.local/include")
if(EXISTS ${NON_ROOT_INC_DIR})
	include_directories(${NON_ROOT_INC_DIR})
endif()

include(verilator)
include(vcs)

# Add CUSTOM_INCLUDE_FILES as target
if(NOT "${EXTRA_INC}" STREQUAL "")
	message(STATUS "Using dynamic library include: ${EXTRA_INC}")
	separate_arguments(EXTRA_INC_LIST NATIVE_COMMAND ${EXTRA_INC})
	file(COPY ${EXTRA_INC_LIST} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	# set_target_properties(
	# 	CUSTOM_INCLUDE_FILES
	# 	PROPERTIES  	
	# 	"${EXTRA_INC_LIST}")
endif()

set_target_properties(
	${ModuleName}
	PROPERTIES CXX_STANDARD 20
						 CXX_STANDARD_REQUIRED ON
						 CXX_EXTENSIONS ON
						 OUTPUT_NAME "UT${ModuleName}")

# Create release directory and Copy output files
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/UT_${ModuleName})
add_custom_command(
	TARGET ${ModuleName}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${ModuleName}>
					${CMAKE_BINARY_DIR}/UT_${ModuleName}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DPI${ModuleName}>
					${CMAKE_BINARY_DIR}/UT_${ModuleName}
	# COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CUSTOM_INCLUDE_FILES>
	# 				${CMAKE_BINARY_DIR}/UT_${ModuleName}
	COMMENT "Copying ${ModuleName} to ${CMAKE_BINARY_DIR}/UT_${ModuleName}")
