cmake_minimum_required(VERSION 3.11)

set(ModuleName {{__TOP_MODULE_NAME__}})
project(${ModuleName} VERSION 1.0)

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

# Non-root install
set(NON_ROOT_INC_DIR "$ENV{HOME}/.local/include")
if(EXISTS ${NON_ROOT_INC_DIR})
	include_directories(${NON_ROOT_INC_DIR})
	link_directories("$ENV{HOME}/.local/lib")
endif()

include(${ModuleName}.cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
add_library(example OBJECT example.cpp)
XSPTarget(
	RTL_MODULE_NAME
	${ModuleName}
	EXECUTABLE_NAME
	UT${ModuleName}_example
	CUSTOM_LIBS
	example
	CUSTOM_LINK_OPTIONS
)