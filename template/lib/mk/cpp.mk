default:
	@cp cpp/dut.cpp ${TARGET}/${TARGET}.cpp 
	@cp cpp/dut.hpp ${TARGET}/${TARGET}.hpp
	@cp cpp/example.cpp ${TARGET}/example.cpp
	@cp cpp/cmake/${SIMULATOR}.cmake ${TARGET}/${PROJECT}.cmake
	@cp cpp/CMakeLists.txt ${TARGET}/CMakeLists.txt
	@cp cpp/Makefile ${TARGET}/Makefile
	@cd ${TARGET} && make
