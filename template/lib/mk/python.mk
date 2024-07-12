default:
	@cp python/example.py ${TARGET}/../example.py
	@cp python/cmake/${SIMULATOR}.cmake ${TARGET}/dut.cmake
	@cp python/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp python/Makefile ${TARGET}/Makefile
	@cp python/dut.i ${TARGET}/dut.i
	@cp python/dut.py ${TARGET}/__init__.py
	@cd ${TARGET} && make
