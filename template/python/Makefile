CURRENT_DIR := $(shell pwd)

all: test clean
	@echo "All complete"

test: compile
	@rm *.a 2>/dev/null || true
	@echo "--------------Comple Success Test Step DUT--------------------"
	@cd ../ && PYTHONPATH=.:{{__XSPCOMM_PYTHON__}}/.. python3 example.py && rm -rf *.fst *.vcd *.fsdb *.log *.key 2>/dev/null || (echo "Error: Test failed, plz check whether behavior is correct" && sleep 10)
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
	cmake . -Bbuild
	cmake --build build --config Debug --parallel `nproc`
	@cp build/_${TARGET}.so .
	@cp build/lib${TARGET}.py .
	@touch ../__init__.py

clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key build

purge: clean
	@rm -rf *.txt Makefile *.hpp *.cmake *.i __pycache__
	@rm ../example.py ../__init__.py || true
	@mv ./* ../ || true
	@echo "purge complete"
	@rm -rf $(CURRENT_DIR)
