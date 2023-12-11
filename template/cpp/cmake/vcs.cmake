add_definitions(-DUSE_VCS)

function(XSPTargetBuild ModuleName)

	# Find VCS
	if(DEFINED ENV{VCS_HOME})
		set(VCS_HOME $ENV{VCS_HOME})
		message(STATUS "Find VCS root: ${VCS_HOME}")
	else()
		message(FATAL_ERROR "Cannot find vcs, please install (vcs)")
	endif()

	# Find Verdi
	if(DEFINED ENV{VERDI_HOME})
		set(VERDI_HOME $ENV{VERDI_HOME})
		message(STATUS "Find Verdi root: ${VERDI_HOME}")
	else()
		message(FATAL_ERROR "Cannot find verdi, please install (verdi)")
	endif()

	# Add VCS link options and libraries
	include_directories(${VCS_HOME}/include ${VCS_HOME}/linux64/lib/)
	add_library(vcs_tls OBJECT IMPORTED)
	set_target_properties(vcs_tls PROPERTIES IMPORTED_OBJECTS
																					 ${VCS_HOME}/linux64/lib/vcs_tls.o)
	add_library(vcs_save_restore_new OBJECT IMPORTED)
	set_target_properties(
		vcs_save_restore_new
		PROPERTIES IMPORTED_OBJECTS ${VCS_HOME}/linux64/lib/vcs_save_restore_new.o)

	# Build the cpp wrapper
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(UT${ModuleName} SHARED IMPORTED)
	set_property(
		TARGET UT${ModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_SOURCE_DIR}/libUT${ModuleName}.so)

	# Workaround for VCS
	execute_process(
		COMMAND
			${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_SOURCE_DIR}/libDPI${ModuleName}.so
			${CMAKE_CURRENT_BINARY_DIR}/libDPI${ModuleName}.so)
	add_library(DPI${ModuleName} SHARED IMPORTED)
	set_property(
		TARGET DPI${ModuleName}
		PROPERTY IMPORTED_LOCATION
						 ${CMAKE_CURRENT_BINARY_DIR}/libDPI${ModuleName}.so)

	# Build the test executable
	add_executable(${ModuleName}UT UT_${ModuleName}.cpp example.cpp)
	target_link_libraries(${ModuleName}UT UT${ModuleName} DPI${ModuleName} xspcomm vcs_tls
												vcs_save_restore_new ${CMAKE_DL_LIBS})
	target_link_options(
		${ModuleName}UT
		PRIVATE
		-L./
		-L${VCS_HOME}/linux64/lib
		-Wl,-rpath=./
		-Wl,-rpath=/usr/local/lib
		-Wl,-rpath=${VCS_HOME}/linux64/lib
		-no-pie
		-Wl,--no-as-needed
		-rdynamic
		-Wl,-whole-archive
		-lvcsucli
		-Wl,-no-whole-archive
		-lzerosoft_rt_stubs
		-luclinative
		-lvirsim
		-lerrorinf
		-lsnpsmalloc
		-lvfs
		-lvcsnew
		-lsimprofile
		-ldl
		-lc
		-lm
		-lpthread
		-lnuma)

endfunction()
