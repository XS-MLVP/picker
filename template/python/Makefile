all: compile
	python3 __init__.py
	cd ../ && python3 example.py || true

compile:
	cmake . -Bbuild
	cmake --build build --config Debug --parallel `nproc`
	cp build/_${TARGET}.so .
	cp build/lib${TARGET}.py .

clean:
	rm -rf *.fst *.vcd *.fsdb *.log *.key
