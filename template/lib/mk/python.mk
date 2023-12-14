default:
	@cp python/cmake/${SIMULATOR}.cmake ${TARGET}/dut.cmake
	@cp python/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp python/Makefile ${TARGET}/Makefile
	@cp python/dut.i ${TARGET}/dut.i
	@cp python/dut.py ${TARGET}/{TARGET}.py
	cd ${TARGET} && make && make clean

ifeq ($(VERBOSE), OFF)
	rm -rf CMakeLists.txt dut* *.hpp build Makefile
endif
