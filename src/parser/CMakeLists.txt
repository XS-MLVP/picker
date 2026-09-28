include_directories(../../include)
include_directories(../../include/codegen)
include_directories(../../include/parser)
set(source_file 
     sv.cpp
     internalcfg.cpp
     uvm.cpp
     verilator_root.cpp
     firrtl.cpp
     gsim_root.cpp
     parser_map.cpp
     verilator_trace/common.cpp
     verilator_trace/version.cpp
     verilator_trace/v5_026.cpp
     verilator_trace/v5_048.cpp
     verilator_trace/dispatch.cpp
)

add_library(parser ${source_file})
target_link_libraries(parser PUBLIC slang::slang)
