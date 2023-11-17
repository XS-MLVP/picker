include_directories(../../include)
include_directories(../../include/codegen)
include_directories(../../include/parse)
set(source_file 
     cpp.cpp
     sv.cpp
     make.cpp
)

add_library(codegen ${source_file})
