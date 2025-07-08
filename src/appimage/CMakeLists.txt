include_directories(../../include)
include_directories(../../include/codegen)
include_directories(../../include/parse)

set(source_file 
    configuration.cpp
)
add_library(appimage ${source_file})
