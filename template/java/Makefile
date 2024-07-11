all: test clean
	@echo "All complete"

test: compile
	@echo "--------------Comple Success Test Step DUT--------------------"
	@java -cp xspcomm-java.jar:${TARGET}-java.jar -ea com.ut.example
	@echo "--------------------------------------------------------------"

compile:
	@cp {{__XSPCOMM_JAR__}} ./
	cmake . -Bbuild
	cmake --build build --config Debug --parallel `nproc`
	
clean:
	@ls|grep -v ".jar"|xargs rm -rf
