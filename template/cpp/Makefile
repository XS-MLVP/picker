all: test clean
	@echo "All complete"

test: compile
	@cp build/UT{{__TOP_MODULE_NAME__}}_example .
	@rm -rf build
	@echo "--------------Comple Success Test Step DUT--------------------"
	@cd ../ && ./UT_{{__TOP_MODULE_NAME__}}/UT{{__TOP_MODULE_NAME__}}_example && rm -rf *.fst *.vcd *.fsdb *.log *.key 2>/dev/null || true
	@echo "--------------------------------------------------------------"

compile:
	@cmake . -Bbuild
	@cmake --build build --config Release --parallel `nproc`

clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key *.dat
