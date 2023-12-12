cmake_minimum_required(VERSION 3.11)

set(ModuleName {{__TOP_MODULE_NAME__}})
project(${ModuleName} VERSION 1.0)

include(${ModuleName}.cmake)
add_library(example OBJECT example.cpp)
XSPTarget(
	RTL_MODULE_NAME
	${ModuleName}
	EXECUTABLE_NAME
	UT${ModuleName}_example
	CUSTOM_LIBS
	example
	CUSTOM_LINK_OPTIONS
)