include_directories(../../include)
include_directories(../../include/codegen)
include_directories(../../include/parse)
set(source_file
     array.cpp
     cpp.cpp
     python.cpp
     java.cpp
     scala.cpp
     golang.cpp
     sv/sv.cpp
     sv/aggregate.cpp
     sv/waveform.cpp
     lib.cpp
     lua.cpp
     uvm.cpp
     mem_direct.cpp
     firrtl.cpp
)

add_library(codegen_ STATIC ${source_file})
