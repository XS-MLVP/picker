include_directories(../include)

set(source_file 
  ../src/gen_python.cpp
)
add_executable(test_templategen ./test_templategen.cpp ${source_file})
