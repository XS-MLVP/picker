# get git hash
macro(get_git_hash _git_hash)
	find_package(Git QUIET)
	if(GIT_FOUND)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
			OUTPUT_VARIABLE ${_git_hash}
			OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	endif()
endmacro()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
							 ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# get git branch
macro(get_git_branch _git_branch)
	find_package(Git QUIET)
	if(GIT_FOUND)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
			OUTPUT_VARIABLE ${_git_branch}
			OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	endif()
endmacro()

# get git dirty
macro(get_git_dirty _git_dirty)
	find_package(Git QUIET)
	if(GIT_FOUND)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} diff --quiet
			RESULT_VARIABLE _git_dirty
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
		if(_git_dirty)
			set(${_git_dirty} "-dirty")
			message(STATUS "GIT is DIRTY: ${${_git_dirty}}")
		else()
			set(${_git_dirty} "")
			message(STATUS "GIT not DIRTY: ${${_git_dirty}}")
		endif()
	endif()
endmacro()

get_git_branch(GIT_BRANCH)
get_git_hash(GIT_HASH)
get_git_dirty(GIT_DIRTY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
							 ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp)
