# cmake for mcv

cmake_minimum_required(VERSION 3.11)
project(mcv VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(is_top_level_project TRUE)
else()
    set(is_top_level_project FALSE)
endif()

# check verilator
execute_process(
    COMMAND bash -c "verilator -V|grep ROOT|grep verilator|awk '{print $3}'"
    OUTPUT_VARIABLE verilator_root
)

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

# third
# add_subdirectory(third/type_safe)
# add_subdirectory(third/cppast)
add_subdirectory(third/yaml-cpp)
# LINK_LIBRARIES(cppast)
LINK_LIBRARIES(yaml-cpp)


# source
add_subdirectory(src)
add_subdirectory(test)

# install
set(CMAKE_INSTALL_PREFIX /usr/local)
set(CMAKE_INSTALL_LOCAL_ONLY TRUE)

install(
    TARGETS mcv DESTINATION bin
)
install(
    DIRECTORY template DESTINATION /etc/mcv
)