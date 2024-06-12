include_directories(../../include)
include_directories(../../include/codegen)
include_directories(../../include/parse)
set(source_file 
     sv.cpp
     internalcfg.cpp
     uvm.cpp
)

add_library(parser ${source_file})