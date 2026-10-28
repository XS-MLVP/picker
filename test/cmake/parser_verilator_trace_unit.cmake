add_executable(test_parser_verilator_trace_unit test_parser_verilator_trace_unit.cpp)
target_link_libraries(test_parser_verilator_trace_unit parser test_common)

# Release builds must execute assertions in these correctness tests.
target_compile_options(test_parser_verilator_trace_unit PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>)
add_executable(test_parser_verilator_trace_dtype_unit test_parser_verilator_trace_dtype_unit.cpp)
target_link_libraries(test_parser_verilator_trace_dtype_unit parser test_common)
target_compile_options(test_parser_verilator_trace_dtype_unit PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>)
add_executable(test_parser_verilator_trace_dtype_array_unit test_parser_verilator_trace_dtype_array_unit.cpp)
target_link_libraries(test_parser_verilator_trace_dtype_array_unit parser test_common)
target_compile_options(test_parser_verilator_trace_dtype_array_unit PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>)
add_executable(test_parser_verilator_trace_structure_unit test_parser_verilator_trace_structure_unit.cpp)
target_link_libraries(test_parser_verilator_trace_structure_unit parser test_common)
target_compile_options(test_parser_verilator_trace_structure_unit PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-UNDEBUG>)
