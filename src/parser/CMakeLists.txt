include_directories(../include)
set(source_file 
     mcv.cpp
     gen_python.cpp
     gen_java.cpp
     gen_golang.cpp
     gen_cpp.cpp
     cpp_parser.cpp
     global.cpp
)

add_executable(${PROJECT_NAME} ${source_file})
