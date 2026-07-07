if(SIMULATOR STREQUAL "vcs")

	add_definitions(-DUSE_VCS)

	# Find VCS
	if(DEFINED ENV{VCS_HOME})
		set(VCS_HOME $ENV{VCS_HOME})
		message(STATUS "Find VCS root: ${VCS_HOME}")
	else()
		message(FATAL_ERROR "Cannot find ENV{VCS_HOME}, please install (vcs)")
	endif()

	# Find Verdi
	if(DEFINED ENV{VERDI_HOME})
		set(VERDI_HOME $ENV{VERDI_HOME})
		message(STATUS "Find Verdi root: ${VERDI_HOME}")
	else()
		message(FATAL_ERROR "Cannot find ENV{VERDI_HOME}, please install (verdi) or set ENV{VERDI_HOME}")
	endif()

	# Add VCS include path
	include_directories(${VCS_HOME}/include ${VCS_HOME}/linux64/lib)

	# Add VCS magic
	set(CMAKE_EXE_LINKER_FLAGS
			"${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,stack-size=1048576")

	# Copy all source files to build directory
	file(GLOB_RECURSE SOURCES "*.sv" "*.v" "*.f")
	file(COPY ${SOURCES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

	# CFLAGS Detect IF CFLAGS is not empty, add -cflags to VCS compile
	set(SIMULATOR_CFLAGS "")
	if(NOT "${CFLAGS}" STREQUAL "")
		# add -CFLAGS to VCS compile
		set(SIMULATOR_CFLAGS "-CFLAGS;${CFLAGS}")
		message(STATUS "VCS CFLAGS: ${CFLAGS}")
	endif()
	
	# if vpi is enabled, add SIMULATOR_FLAGS '+vpi' and '-debug_access+all'
	if(${VPI} STREQUAL "ON")
		set(SIMULATOR_FLAGS "${SIMULATOR_FLAGS};+vpi;-debug_access+all")
	endif()

	if(${COVERAGE} STREQUAL "ON")
		add_definitions(-DVCS_COVERAGE)
		set(VCS_HAS_COVERAGE_FLAGS OFF)
		set(VCS_HAS_COVERAGE_DIR OFF)
		set(VCS_HAS_COVERAGE_NAME OFF)
		foreach(flag IN LISTS SIMULATOR_FLAGS)
			if(flag MATCHES "^-cm$" OR flag MATCHES "^-cm=")
				set(VCS_HAS_COVERAGE_FLAGS ON)
			elseif(flag STREQUAL "-cm_dir")
				set(VCS_HAS_COVERAGE_DIR ON)
			elseif(flag STREQUAL "-cm_name")
				set(VCS_HAS_COVERAGE_NAME ON)
			endif()
		endforeach()
		if(NOT VCS_HAS_COVERAGE_FLAGS)
			set(SIMULATOR_FLAGS "${SIMULATOR_FLAGS};-cm;line+cond+fsm+tgl+branch+assert")
		endif()
		if(NOT VCS_HAS_COVERAGE_DIR)
			set(SIMULATOR_FLAGS "${SIMULATOR_FLAGS};-cm_dir;${ModuleName}.vdb")
		endif()
		if(NOT VCS_HAS_COVERAGE_NAME)
			set(SIMULATOR_FLAGS "${SIMULATOR_FLAGS};-cm_name;${ModuleName}")
		endif()
	endif()

	# Using Compiled vcs dynamic library
	if(NOT "${VCS_DYN}" STREQUAL "")
		message(STATUS "Using VCS dyn target: ${VCS_DYN}")
		file(COPY ${VCS_DYN} ${VCS_DYN}.daidir DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
		# Extract Include path has been added by ${EXTRA_INC_LIST} and check if it is empty
		if("${EXTRA_INC_LIST}" STREQUAL "")
			message(WARNING "EXTRA_INC_LIST is empty, please check your CMakeLists.txt")
		endif()
	else()
	# Using VCS compile

	# ---------- Verdi PLI selection ----------
	# The exported --verdi-mode value is rendered into this template.
	# modern: use -debug_access+all for Verdi >= 2024.09.
	# legacy: use the older novas.tab interface.
{% if __VERDI_MODE__ == "modern" %}
	message(STATUS "Verdi PLI: modern (-debug_access+all, from --verdi-mode modern)")
	set(_VCS_VERDI_FLAGS "-debug_access+all")
{% else %}
	message(STATUS "Verdi PLI: legacy (-P novas.tab, default; use --verdi-mode modern for Verdi >= 2024.09)")
	set(_VCS_VERDI_FLAGS "-P;${VERDI_HOME}/share/PLI/VCS/LINUX64/novas.tab;-P;pli.tab")
{% endif %}
	# pli.a is required in both modes to pull in the VCS runtime symbols.
	set(_VCS_VERDI_LIBS "${VERDI_HOME}/share/PLI/VCS/LINUX64/pli.a")
	# ---------------------------------------------------

		execute_process(
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND
				vcs -e VcsMain -slave ${VCS_TRACE} -sverilog -lca -l compile.log
				-top ${ModuleName}_top -full64 -timescale=1ns/1ps
				${ModuleName}_top.sv ${ModuleName}.v -f filelist.f
				-o libDPI${ModuleName}.so +modelsave -LDFLAGS "-shared"
				${SIMULATOR_FLAGS}
				${SIMULATOR_CFLAGS}
				${_VCS_VERDI_FLAGS}
				${_VCS_VERDI_LIBS})
	endif()

	# Add VCS dependency library
	add_library(vcs_tls OBJECT IMPORTED)
	set_target_properties(vcs_tls PROPERTIES IMPORTED_OBJECTS ${VCS_HOME}/linux64/lib/vcs_tls.o)
	add_library(vcs_save_restore OBJECT IMPORTED)
	set_target_properties(
		vcs_save_restore PROPERTIES IMPORTED_OBJECTS ${VCS_HOME}/linux64/lib/vcs_save_restore.o)

	# Set VCS DPI library as imported library
	link_directories(${CMAKE_CURRENT_SOURCE_DIR})
	add_library(DPI${ModuleName} SHARED IMPORTED)
	set_target_properties(DPI${ModuleName} PROPERTIES IMPORTED_LOCATION libDPI${ModuleName}.so)

	# Compile libUT wrapper with VCS DPI library
	include_directories(${CMAKE_CURRENT_BINARY_DIR})
	add_library(${ModuleName} SHARED dut_base)
	target_link_libraries(${ModuleName} PRIVATE DPI${ModuleName} vcs_tls vcs_save_restore)
	target_link_options(${ModuleName} PRIVATE -Wl,-rpath,./)

	# Copy libDPI${ModuleName}.so.daidir directory to build directory
	add_custom_command(
		TARGET ${ModuleName}
		POST_BUILD
		COMMAND
			${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_BINARY_DIR}/libDPI${ModuleName}.so.daidir
			${CMAKE_BINARY_DIR}/UT_${ModuleName}/libDPI${ModuleName}.so.daidir
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/vc_hdrs.h
						${CMAKE_BINARY_DIR}/UT_${ModuleName}/UT_${ModuleName}_dpi.hpp
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/vc_hdrs.h
						${CMAKE_BINARY_DIR}/UT_${ModuleName}/vc_hdrs.h)

endif()
