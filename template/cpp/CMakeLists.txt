cmake_minimum_required(VERSION 3.11)

set(ModuleName {{__TOP_MODULE_NAME__}})
project(${ModuleName} VERSION 1.0)

include(${ModuleName}.cmake)

xsptargetbuild(${ModuleName})

set_target_properties(
	${ModuleName}UT
	PROPERTIES CXX_STANDARD 20
						 CXX_STANDARD_REQUIRED ON
						 CXX_EXTENSIONS ON
						 OUTPUT_NAME "UT${ModuleName}_example")
