export BUILD_XSPCOMM_SWIG?=python

all: clean build

init:
	rm -rf dependence/xcomm
	git clone https://github.com/XS-MLVP/xcomm.git dependence/xcomm 
	# checkout the same branch as the parent project
	@PARENT_BRANCH=`git branch --show-current` && cd dependence/xcomm && git checkout $$PARENT_BRANCH || echo "there is no branch $$PARENT_BRANCH in xcomm"
	
build:
	@if ! command -v verible-verilog-format ; then \
		echo "verible-verilog-format could not be found"; \
		echo "install verible-verilog-format"; \
    	echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"; \
		exit 1; \
	fi
	cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL=`nproc` $(ARGS)
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

test_vpi_all:
	bash example/InternalSignals/release-verilator.sh --lang cpp
	bash example/InternalSignals/release-verilator.sh --lang golang
	bash example/InternalSignals/release-verilator.sh --lang java
	bash example/InternalSignals/release-verilator.sh --lang lua
	bash example/InternalSignals/release-verilator.sh --lang python
	bash example/InternalSignals/release-verilator.sh --lang scala

test_mem_direct_all:
	bash example/CacheSignalCFG/release-verilator.sh --lang cpp
	bash example/CacheSignalCFG/release-verilator.sh --lang golang
	bash example/CacheSignalCFG/release-verilator.sh --lang java
	bash example/CacheSignalCFG/release-verilator.sh --lang lua
	bash example/CacheSignalCFG/release-verilator.sh --lang python
	bash example/CacheSignalCFG/release-verilator.sh --lang scala

test_all_java:
	bash example/Adder/release-verilator.sh --lang java
	bash example/RandomGenerator/release-verilator.sh --lang java
	bash example/DualPortStackCb/release-verilator.sh --lang java
	bash example/InternalSignals/release-verilator.sh --lang java
	bash example/CacheSignalCFG/release-verilator.sh --lang java

test_all_scala:
	bash example/Adder/release-verilator.sh --lang scala
	bash example/RandomGenerator/release-verilator.sh --lang scala
	bash example/DualPortStackCb/release-verilator.sh --lang scala
	bash example/InternalSignals/release-verilator.sh --lang scala
	bash example/CacheSignalCFG/release-verilator.sh --lang scala

clean:
	rm -rf temp build dist picker_out*

wheel: clean
	cd dependence/xcomm && make wheel
	NO_BUILD_XSPCOMM=1 pipx run build
	cp dependence/xcomm/dist/* ./dist

wheel_install: wheel
	pip uninstall -y xspcomm picker
	pip install dist/xspcomm*.whl dist/picker*.whl