export BUILD_XSPCOMM_SWIG?=python

all: clean build

init:
	rm -rf dependence/xcomm
	git clone https://github.com/XS-MLVP/xcomm.git dependence/xcomm
	cd dependence/xcomm && git checkout master
	
build:
	@if ! command -v verible-verilog-format ; then \
		echo "verible-verilog-format could not be found"; \
		echo "install verible-verilog-format"; \
    	echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"; \
		exit 1; \
	fi
	cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release $(ARGS)
	cd build && make -j`nproc`

install: build
	cd build && make install

test: build
	./build/bin/picker -h
	./build/bin/picker pack -h
	./build/bin/picker exports -h

test_all:
	rm -rf picker_out_*
	./example/Adder/release-verilator.sh --lang python
	./example/RandomGenerator/release-verilator.sh --lang python
	./example/AdderMultiInstance/release-verilator.sh --lang python
	./example/Cache/release-verilator.sh --lang python

clean:
	rm -rf temp build dist picker_out*

wheel: clean
	cd dependence/xcomm && make wheel
	NO_BUILD_XSPCOMM=1 pipx run build
	cp dependence/xcomm/dist/* ./dist

wheel_install: wheel
	pip uninstall -y xspcomm picker
	pip install dist/xspcomm*.whl dist/picker*.whl