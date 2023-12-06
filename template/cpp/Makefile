all: compile
	cp build/UT{{__TOP_MODULE_NAME__}}_example .
	rm -rf build
	./UT{{__TOP_MODULE_NAME__}}_example || true

compile:
	cmake . -Bbuild -DSIMULATOR=$(SIMULATOR) -DTRACE=$(TRACE) -DPROJECT=$(PROJECT)
	cmake --build build --config Release --parallel `nproc`

clean: 
