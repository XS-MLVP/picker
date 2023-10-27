include_directories(../../include)
include_directories(../../include/codegen)
set(source_file 
     cpp.cpp
)

add_library(codegen ${source_file})
