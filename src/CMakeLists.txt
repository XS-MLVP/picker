include_directories(../include)
set(source_file 
     mcv.cpp
)

add_subdirectory(./codegen)
add_subdirectory(./parser)

add_executable(${PROJECT_NAME} ${source_file})
target_link_libraries(${PROJECT_NAME} codegen parser)
target_link_options(${PROJECT_NAME} PRIVATE "-static")


