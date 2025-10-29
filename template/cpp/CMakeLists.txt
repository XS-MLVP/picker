cmake_minimum_required(VERSION 3.11)

set(ModuleName {{__TOP_MODULE_NAME__}})
project(${ModuleName} VERSION 1.0)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fcoroutines -ftls-model=global-dynamic")
	add_definitions(-DENABLE_XCOROUTINE=true)
	set(ENABLE_XCOROUTINE true)
else()
    message(WARNING "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. If you need coroutines, please use a different C++ compiler.")
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -ftls-model=global-dynamic")
	add_definitions(-DENABLE_XCOROUTINE=false)
	set(ENABLE_XCOROUTINE false)
endif()

# Non-root install
# set rpath
set(CMAKE_BUILD_RPATH "")
if(CMAKE_SYSTEM_NAME MATCHES "Darwin") # macOS
	list(APPEND CMAKE_BUILD_RPATH "@loader_path")
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	list(APPEND CMAKE_BUILD_RPATH "$ORIGIN")
endif()
set(NON_ROOT_INC_DIR "$ENV{HOME}/.local/include")
if(EXISTS ${NON_ROOT_INC_DIR})
	include_directories(${NON_ROOT_INC_DIR})
	link_directories("$ENV{HOME}/.local/lib")
endif()

# libs find by picker
include_directories({{__XSPCOMM_INCLUDE__}})
link_directories({{__XSPCOMM_LIB__}})

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