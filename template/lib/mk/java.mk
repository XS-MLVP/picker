default:
ifeq ($(EXAMPLE), ON)
	@cp java/example.java ${TARGET}/../example.java
endif
	@cp java/cmake/${SIMULATOR}.cmake ${TARGET}/dut.cmake
	@cp java/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp java/Makefile ${TARGET}/Makefile
	@cp java/dut.i ${TARGET}/dut.i
	@cp java/dut.java ${TARGET}/${TARGET}.java
	@cp java/MANIFEST.MF ${TARGET}/MANIFEST.MF
	@cp /usr/local/share/picker/java/xspcomm-java.jar ${TARGET}/
	@cd ${TARGET} && make

ifeq ($(VERBOSE), OFF)
	@cd ${TARGET} && make clean
	@cd ${TARGET} && rm -rf CMakeLists.txt dut* *.hpp build Makefile __pycache__ *.cmake
endif

