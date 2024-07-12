default:
	@cp golang/example.go ${TARGET}/../example.go
	@cp golang/cmake/${SIMULATOR}.cmake ${TARGET}/dut.cmake
	@cp golang/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp golang/Makefile ${TARGET}/Makefile
	@cp golang/dut.i ${TARGET}/dut.i
	@cp golang/dut.go ${TARGET}/${TARGET}_Wrapper.go
	@cp golang/go.mod ${TARGET}/go.mod
	@cd ${TARGET} && make
