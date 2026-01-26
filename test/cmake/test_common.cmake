# This file defines the test_common library,
# which provides common stubs and definitions for tests
# that require symbols from the main application logic
# but should not be linked against the full application.

add_library(test_common ${CMAKE_CURRENT_SOURCE_DIR}/test_common.cpp)
