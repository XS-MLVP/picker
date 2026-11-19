add_executable(test_parser_verilator_root_unit test_parser_verilator_root_unit.cpp)
target_link_libraries(test_parser_verilator_root_unit parser test_common)

# This test states its expectations with assert(), which -DNDEBUG would discard.
target_compile_options(test_parser_verilator_root_unit PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>)
target_include_directories(test_parser_verilator_root_unit PRIVATE ${PROJECT_SOURCE_DIR}/src/parser)
