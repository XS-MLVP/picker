include_directories(../include)

set(source_file 
  ../src/gen_python.cpp
  ../src/cpp_parser.cpp
  ../src/global.cpp
)
add_executable(test_templategen ./test_templategen.cpp ${source_file})
