CURRENT_DIR := $(shell pwd)
NPROC ?= $(shell (nproc 2>/dev/null || sysctl -n hw.ncpu) 2>/dev/null)
.PHONY: all test compile clean purge copy_xspcomm

all:
	@$(MAKE) test; result=$$?; $(MAKE) clean; exit $$result
	@echo "All complete"

test: compile
	@rm *.a 2>/dev/null || true
	@echo "--------------Compile Success Test Step DUT--------------------"
	@cd ../ && PYTHONPATH=.:{{__XSPCOMM_PYTHON__}}/.. python3 example.py || \
		{ result=$$?; echo "Error: Test failed, plz check whether behavior is correct"; exit $$result; }
	@echo "--------------------------------------------------------------"

# copy xspcomm lib
copy_xspcomm_lib={{__COPY_XSPCOMM_LIB__}}
ifneq ($(copy_xspcomm_lib), true)
copy_xspcomm:
	@echo "Ignore copy python module: xspcomm"
else
copy_xspcomm:
	@echo "Try copy python module: xspcomm"
	@cp -r {{__XSPCOMM_PYTHON__}} xspcomm
endif

compile: copy_xspcomm
	@echo "start build wrapper at" && date
	cmake . -Bbuild
	cmake --build build --config Debug --parallel $(NPROC)
	@cp build/_${TARGET}.so .
	@cp build/lib${TARGET}.py .
	@touch ../__init__.py
	@echo "end build wrapper at" && date

clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key build ../*.fst ../*.vcd ../*.fsdb ../*.log ../*.key

purge: clean
	@rm -rf *.txt Makefile *.hpp *.cmake *.i __pycache__
	@rm ../__init__.py || true
	@find . -mindepth 1 -maxdepth 1 -not -name '*.vdb' -exec sh -c 'for path in "$$@"; do mv "$$path" ../ || exit 1; done' sh {} +
	@cd .. && rm -rf "$(CURRENT_DIR)" && echo "purge complete"
