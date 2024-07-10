all: compile
	@echo "--------------Comple Success Test Step DUT--------------------"
	@scala -cp xspcomm-scala.jar:${TARGET}-scala.jar -ea com.ut.example
	@echo "--------------------------------------------------------------"

compile:
	@cp {{__XSPCOMM_JAR__}} ./
	cmake . -Bbuild
	cmake --build build --config Debug --parallel `nproc`
	
clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key
	@ls|grep -v ".jar"|grep -v "Makefile"|grep -v "CMakeLists.txt"|xargs rm -rf
