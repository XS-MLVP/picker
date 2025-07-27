if(SIMULATOR STREQUAL "gsim")
	message(STATUS "gsim simulator generate source files")
	set(SIMULATOR_FLAGS "$ENV{GSIM_FLAGS}")
	# find cmd gsim and generate source files
	find_program(GSIM_EXECUTABLE gsim)
	if(NOT GSIM_EXECUTABLE)
		message(FATAL_ERROR "gsim executable not found, please install gsim or set GSIM_EXECUTABLE to the path")
	endif()
	set(GSIM_EXECUTABLE ${GSIM_EXECUTABLE} CACHE FILEPATH "Path to gsim executable")
	message(STATUS "Using gsim executable: ${GSIM_EXECUTABLE} in workspace ${CMAKE_CURRENT_BINARY_DIR}")
	message(STATUS "SOURCE DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/_SRC)
	execute_process(
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND
				${GSIM_EXECUTABLE} ${SIMULATOR_FLAGS} --dir=_SRC
				${CMAKE_CURRENT_SOURCE_DIR}/${ModuleName}.fir
			COMMAND_ECHO STDOUT
			)
	include_directories(${CMAKE_CURRENT_BINARY_DIR}/_SRC)
	file(GLOB GSIM_GENERATED_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/_SRC/*.cpp")

	find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(GMP QUIET gmp)
        if(GMP_FOUND)
            message(STATUS "Found GMP via pkg-config: ${GMP_VERSION}")
            include_directories(${GMP_INCLUDE_DIRS})
            link_directories(${GMP_LIBRARY_DIRS})
            message(STATUS "Found GMP include: ${GMP_INCLUDE_DIRS}, lib: ${GMP_LIBRARY_DIRS}")
		endif()
    endif()
    
    # Fallback to manual search if pkg-config didn't find it
    if(NOT GMP_FOUND)
        find_path(GMP_INCLUDE_DIRS gmp.h)
        find_library(GMP_LIBRARIES NAMES gmp)
        if(GMP_INCLUDE_DIRS AND GMP_LIBRARIES)
            set(GMP_FOUND TRUE)
            include_directories(${GMP_INCLUDE_DIRS})
	        get_filename_component(GMP_LIBRARY_DIRS ${GMP_LIBRARIES} DIRECTORY)
    	    link_directories(${GMP_LIBRARY_DIRS})
            message(STATUS "Found GMP include: ${GMP_INCLUDE_DIRS}, lib: ${GMP_LIBRARY_DIRS}")
		else()
            message(FATAL_ERROR "GMP library not found")
        endif()
    endif()

	add_library(${ModuleName}
	STATIC
	dut_base.cpp
	${GSIM_GENERATED_SOURCES}
	)

endif()