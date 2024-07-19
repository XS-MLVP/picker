# cmake for picker

cmake_minimum_required(VERSION 3.11)
project(picker VERSION 0.1.0)

unset(CMAKE_MODULE_PATH)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(Utility)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(is_top_level_project TRUE)
else()
    set(is_top_level_project FALSE)
endif()

# check verilator
if ("$ENV{VERILATOR_ROOT}" STREQUAL "")
    execute_process(
        COMMAND bash -c "verilator -V|grep ROOT|grep verilator|head -n 1|awk '{print $3}'"
        OUTPUT_VARIABLE verilator_root
    )
else()
    set(verilator_root $ENV{VERILATOR_ROOT})
endif()

if (NOT "${verilator_root}" STREQUAL "")
    include_directories(${verilator_root})
endif()

# options
option(CPPAST_BUILD_TEST "whether or not to build the tests" OFF)
option(CPPAST_BUILD_EXAMPLE "whether or not to build the examples" OFF)
option(CPPAST_BUILD_TOOL "whether or not to build the tool" OFF)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

#dependencies
add_subdirectory(dependence/xcomm)

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
    TARGETS picker DESTINATION bin
)
install(
    DIRECTORY template DESTINATION share/picker
)
