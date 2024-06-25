default:
	@cp java/example.java ${TARGET}/../example.java
	@cp java/cmake/${SIMULATOR}.cmake ${TARGET}/dut.cmake
	@cp java/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp java/Makefile ${TARGET}/Makefile
	@cp java/dut.i ${TARGET}/dut.i
	@cp java/dut.java ${TARGET}/${TARGET}.java
	@cp java/MANIFEST.MF ${TARGET}/MANIFEST.MF
	@cd ${TARGET} && make
