include_directories(../include)
set(source_file mcv.cpp gen_python.cpp cpp_parser.cpp)
add_executable(${PROJECT_NAME} ${source_file})
