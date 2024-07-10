default:
	@cp scala/example.scala ${TARGET}/../example.scala
	@cp scala/cmake/${SIMULATOR}.cmake ${TARGET}/dut.cmake
	@cp scala/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp scala/Makefile ${TARGET}/Makefile
	@cp scala/dut.i ${TARGET}/dut.i
	@cp scala/dut.java ${TARGET}/Java${TARGET}.scala
	@cp scala/dut.scala ${TARGET}/${TARGET}.scala
	@cp scala/MANIFEST.MF ${TARGET}/MANIFEST.MF
	@cd ${TARGET} && make
