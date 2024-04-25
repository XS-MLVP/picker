all: compile
	cp build/UT{{__TOP_MODULE_NAME__}}_example .
	rm -rf build
	cd ../ && ./UT_{{__TOP_MODULE_NAME__}}/UT{{__TOP_MODULE_NAME__}}_example || true

compile:
	cmake . -Bbuild
	cmake --build build --config Release --parallel `nproc`

clean: 
	rm -rf *.fst *.vcd *.fsdb *.log *.key *.dat