all: compile
	@echo "--------------Comple Success Test Step DUT--------------------"
	@java -cp xspcomm-java.jar:${TARGET}.jar com.ut.example
	@echo "--------------------------------------------------------------"

compile:
	@cp {{__XSPCOMM_JAR__}} ./
	cmake . -Bbuild
	cmake --build build --config Debug --parallel `nproc`
	
clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key
	@ls|grep -v ".jar"|grep -v "Makefile"|xargs rm -rf
