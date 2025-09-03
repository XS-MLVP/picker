include_directories(../include)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(parser_internalcfg)
include(parser_verilator_root)
include(picker_utils)

enable_testing()

add_test(NAME test_picker_utils COMMAND test_picker_utils)
add_test(NAME test_parser_internalcfg COMMAND test_parser_internalcfg)
add_test(NAME test_parser_verilator_root COMMAND test_parser_verilator_root)
