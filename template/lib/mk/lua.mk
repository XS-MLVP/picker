 .PHONY: default
default:
	@cp lua/example.lua ${TARGET}/../example.lua
	@cp lua/cmake/${SIMULATOR}.cmake ${TARGET}/dut.cmake
	@cp lua/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp lua/Makefile ${TARGET}/Makefile
	@cp lua/dut.i ${TARGET}/dut.i
	@cp lua/dut.lua ${TARGET}/${TARGET}.lua
	@cd ${TARGET} && make
