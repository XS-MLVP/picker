cmake_minimum_required(VERSION 3.15)
project({{__TOP_MODULE_NAME__}} VERSION 0.0.1)

include(CheckIncludeFile)
check_include_file("execinfo.h" HAVE_EXECINFO_H)
if(HAVE_EXECINFO_H)
    add_definitions(-DHAVE_EXECINFO_H)
else()
    message(WARNING "The <execinfo.h> not find, xutil.Assert will not print the call stack.")
endif()

function(init_xspcomm)
set(XSPCOMM_VERSION "${PROJECT_VERSION}" PARENT_SCOPE)
add_definitions(-DXSPCOMM_VERSION="${PROJECT_VERSION}")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_STANDARD 20 PARENT_SCOPE)
	set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fcoroutines -ftls-model=global-dynamic" PARENT_SCOPE)
	add_definitions(-DENABLE_XCOROUTINE=true)
	set(ENABLE_XCOROUTINE true PARENT_SCOPE)
else()
    message(WARNING "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. If you need coroutines, please use a different C++ compiler.")
	set(CMAKE_CXX_STANDARD 17 PARENT_SCOPE)
	set(CMAKE_CXX_STANDARD_REQUIRED ON PARENT_SCOPE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -ftls-model=global-dynamic" PARENT_SCOPE)
	add_definitions(-DENABLE_XCOROUTINE=false)
	set(ENABLE_XCOROUTINE false PARENT_SCOPE)
endif()
endfunction()

init_xspcomm()

# Non-root install
set(CMAKE_BUILD_RPATH $ORIGIN)
set(NON_ROOT_INC_DIR "$ENV{HOME}/.local/include")
if(EXISTS ${NON_ROOT_INC_DIR})
	include_directories(${NON_ROOT_INC_DIR})
    link_directories("$ENV{HOME}/.local/lib")
endif()

# SWIG Initialization
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_SWIG_FLAGS "-c++")

if(POLICY CMP0078)
    message("Find CMP0078, set it be NEW")
    cmake_policy(SET CMP0078 NEW)
endif()
if(POLICY CMP0086)
    message("Find CMP0086, set it be NEW")
    cmake_policy(SET CMP0086 NEW)
endif()
if(POLICY CMP0148)
    message("Find CMP0148, set it be NEW")
    cmake_policy(SET CMP0148 NEW)
endif()

find_package(SWIG 4.2.0 REQUIRED)
include(${SWIG_USE_FILE})

include_directories(./)
set(SWIG_OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}/_goswig)

# Python Initialization

find_package(Python COMPONENTS Interpreter Development)
include_directories(${Python_INCLUDE_DIRS})

link_directories({{__XSPCOMM_LIB__}})

# Init swig module
include(dut.cmake)
XSGolangTarget(
    RTL_MODULE_NAME ${PROJECT_NAME}
)