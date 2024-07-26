include_directories(../include)
set(source_file 
     picker.cpp
)

add_subdirectory(./codegen)
add_subdirectory(./parser)

add_executable(${PROJECT_NAME} ${source_file})
target_link_libraries(${PROJECT_NAME} codegen parser)


search_libs(ALL_FOUND LIBS libm.a libc.a)
if (ALL_FOUND)
     target_link_options(${PROJECT_NAME} PRIVATE "-static")
else()
    message(WARNING "picker cannot be built statically")
endif()
