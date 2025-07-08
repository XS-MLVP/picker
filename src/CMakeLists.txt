include_directories(../include)

add_subdirectory(./codegen)
add_subdirectory(./parser)
add_subdirectory(./appimage)

# executable
set(source_file 
    picker.cpp
)
message(STATUS "PROJECT_NAME: ${PROJECT_NAME}")
add_executable(${PROJECT_NAME} ${source_file})

target_link_libraries(${PROJECT_NAME} codegen_ parser appimage)

install(
    TARGETS ${PROJECT_NAME} DESTINATION bin
)

search_libs(ALL_FOUND LIBS libm.a libc.a)
if (ALL_FOUND)
     target_link_options(${PROJECT_NAME} PRIVATE "-static")
else()
    message(WARNING "picker cannot be built statically")
endif()


# # Add AppImage target
# add_custom_target(appimage
    