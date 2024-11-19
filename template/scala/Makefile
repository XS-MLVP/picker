CURRENT_DIR := $(shell pwd)

all: test clean
	@echo "All complete"


test: compile
	@echo "--------------Comple Success Test Step DUT--------------------"
	@scala -cp {{__XSPCOMM_JAR__}}:${TARGET}-scala.jar -ea com.ut.example
	@echo "--------------------------------------------------------------"

# copy xspcomm lib
copy_xspcomm_lib={{__COPY_XSPCOMM_LIB__}}
ifneq ($(copy_xspcomm_lib), true)
copy_xspcomm:
	@echo "Ignore copy scala module: xspcomm"
else
copy_xspcomm:
	@echo "Try copy scala module: xspcomm"
	@cp {{__XSPCOMM_JAR__}} ./
endif

compile: copy_xspcomm
	cmake . -Bbuild
	cmake --build build --config Debug --parallel `nproc`
	
clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key *.a build

purge: clean
	@ls|grep -v ".jar"|xargs rm -rf
	@mv ./* ../ || true
	@echo "purge complete"
	@rm -rf $(CURRENT_DIR)
