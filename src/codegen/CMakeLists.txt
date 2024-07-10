include_directories(../../include)
include_directories(../../include/codegen)
include_directories(../../include/parse)
set(source_file 
     cpp.cpp
     python.cpp
     java.cpp
     scala.cpp
     sv.cpp
     lib.cpp
     uvm.cpp
)

add_library(codegen ${source_file})
