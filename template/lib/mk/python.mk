default:
	@cp python/cmake/${SIMULATOR}.cmake ${TARGET}/dut.cmake
	@cp python/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp python/Makefile ${TARGET}/Makefile
	@cp python/dut.i ${TARGET}/dut.i
	@cp python/dut.py ${TARGET}/__init__.py
	@cp -r /usr/local/share/picker/python/xspcomm ${TARGET}/xspcomm
	cd ${TARGET} && make && make clean

ifeq ($(VERBOSE), OFF)
	cd ${TARGET} && rm -rf CMakeLists.txt dut* *.hpp build Makefile __pycache__ *.cmake
endif
