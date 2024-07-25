all: test clean
	@echo "All complete"

test: compile
	@python3 __init__.py
	@rm *.a 2>/dev/null || true
	@echo "--------------Comple Success Test Step DUT--------------------"
	@cd ../ && PYTHONPATH=. python3 example.py && rm -rf *.fst *.vcd *.fsdb *.log *.key 2>/dev/null || (echo "Error: Test failed, plz check whether behavior is correct" && sleep 10)
	@echo "--------------------------------------------------------------"

compile:
	@cp -r {{__XSPCOMM_PYTHON__}} xspcomm
	cmake . -Bbuild
	cmake --build build --config Debug --parallel `nproc`
	@cp build/_${TARGET}.so .
	@cp build/lib${TARGET}.py .

clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key
