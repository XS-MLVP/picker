
add_executable(test_parser_verilator_root test_parser_verilator_root.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/verilator_root/common.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/verilator_root/v5_026.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/verilator_root/v5_054.cpp
    ${PROJECT_SOURCE_DIR}/src/parser/verilator_root/dispatch.cpp)