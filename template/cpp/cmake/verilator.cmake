add_definitions(-DUSE_VERILATOR)

function(XSPTargetBuild ModuleName)

	find_package(verilator REQUIRED)
	include_directories(${VERILATOR_ROOT}/include
											${VERILATOR_ROOT}/include/vltstd)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(UT${ModuleName} SHARED IMPORTED)
	set_property(
		TARGET UT${ModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_SOURCE_DIR}/libUT${ModuleName}.so)
	add_executable(${ModuleName}UT UT_${ModuleName}.cpp example.cpp)
	target_link_libraries(${ModuleName}UT UT${ModuleName} xspcomm ${CMAKE_DL_LIBS})
	target_link_options(${ModuleName}UT PRIVATE -Wl,-rpath=./ -Wl,-rpath=/usr/local/lib)

endfunction()
