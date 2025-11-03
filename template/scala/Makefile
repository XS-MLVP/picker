CURRENT_DIR := $(shell pwd)
export NPROC := $(shell (nproc 2>/dev/null || sysctl -n hw.ncpu) 2>/dev/null)
.PHONY: all test compile clean purge copy_xspcomm

all: test clean
	@echo "All complete"


test: compile
	@echo "--------------Compile Success Test Step DUT--------------------"
	@scala -cp {{__XSPCOMM_JAR__}}:${TARGET}-scala.jar com.ut.example
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
	cmake --build build --config Debug --parallel $(NPROC)
	
clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key *.a build

purge: clean
	@find . -mindepth 1 -maxdepth 1 -not -name "*.jar" -exec rm -rf {} +
	@mv ./* ../ || true
	@echo "purge complete"
	@rm -rf $(CURRENT_DIR)
