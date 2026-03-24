# cmake for picker

cmake_minimum_required(VERSION 3.15)
if(APPLE)
    include(cmake/MacOSHomebrewLLVM.cmake)
endif ()
project(picker VERSION 0.9.0)

unset(CMAKE_MODULE_PATH)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(Utility)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(is_top_level_project TRUE)
else()
    set(is_top_level_project FALSE)
endif()
if(is_top_level_project)
    enable_testing()
endif()

# options
option(CPPAST_BUILD_TEST "whether or not to build the tests" OFF)
option(CPPAST_BUILD_EXAMPLE "whether or not to build the examples" OFF)
option(CPPAST_BUILD_TOOL "whether or not to build the tool" OFF)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

#dependencies
if ("$ENV{NO_BUILD_XSPCOMM}" STREQUAL "")
add_subdirectory(dependence/xcomm)
endif()

# Verilog/SystemVerilog frontend
set(FETCHCONTENT_FULLY_DISCONNECTED ON CACHE BOOL "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_fmt ON CACHE BOOL "" FORCE)

if(EXISTS "${PROJECT_SOURCE_DIR}/dependence/fmt/CMakeLists.txt")
    set(FETCHCONTENT_SOURCE_DIR_FMT "${PROJECT_SOURCE_DIR}/dependence/fmt" CACHE PATH "" FORCE)
else()
    message(FATAL_ERROR "dependence/fmt is missing. Run 'make init' first to fetch fmt.")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/dependence/slang/CMakeLists.txt")
    set(SLANG_INCLUDE_TOOLS OFF CACHE BOOL "" FORCE)
    set(SLANG_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
    set(SLANG_INCLUDE_DOCS OFF CACHE BOOL "" FORCE)
    set(SLANG_INCLUDE_PYLIB OFF CACHE BOOL "" FORCE)
    set(SLANG_INCLUDE_PYTHON_DOCS OFF CACHE BOOL "" FORCE)
    set(SLANG_INCLUDE_INSTALL OFF CACHE BOOL "" FORCE)
    set(SLANG_USE_MIMALLOC OFF CACHE BOOL "" FORCE)
    add_subdirectory(dependence/slang)
else()
    message(FATAL_ERROR "dependence/slang is missing. Run 'make init' first to fetch slang and fmt.")
endif()

# third
# add_subdirectory(third/type_safe)
# add_subdirectory(third/cppast)
add_subdirectory(third/yaml-cpp)
LINK_LIBRARIES(yaml-cpp)

# source
add_subdirectory(src)
add_subdirectory(test)

# install
set(CMAKE_INSTALL_LOCAL_ONLY TRUE)


install(
    DIRECTORY template DESTINATION share/picker
)
