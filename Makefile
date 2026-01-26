SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

.PHONY: all init build install appimage test test_all test_vpi_all test_mem_direct_all \
        test_all_java test_all_scala clean wheel wheel_install tests smoke_tests unit_tests

export NPROC := $(shell (nproc 2>/dev/null || sysctl -n hw.ncpu) 2>/dev/null)
export BUILD_XSPCOMM_SWIG ?= python
verible_arch := $(shell uname -m)
ifneq ($(verible_arch),x86_64)
	verible_arch := $(shell echo $(verible_arch) | sed 's/aarch64/arm64/')
endif
VERIBLE_VERSION ?= v0.0-4007-g98bdb38a
VERIBLE_TGZ := verible-$(VERIBLE_VERSION)-linux-static-$(verible_arch).tar.gz
VERIBLE_URL := https://github.com/chipsalliance/verible/releases/download/$(VERIBLE_VERSION)/$(VERIBLE_TGZ)

# Default target should not destroy previous builds
all: init build

init:
	@bash scripts/init.sh
	
build:
	cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL=$(NPROC) $(ARGS)
	cd build && $(MAKE) -j$(NPROC)

clean_build:
	rm -rf build

rebuild: clean_build build

install: build
	cd build && $(MAKE) install

appimage: init
	rm -rf AppDir
# Reuse existing build if present; otherwise configure and build
	@if [ ! -d build ]; then \
		cmake -DCMAKE_INSTALL_PREFIX=/usr . -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL=$(NPROC) $(ARGS); \
		cd build && $(MAKE) -j$(NPROC); \
	else \
		echo "Using existing build directory for AppImage"; \
	fi
# Install into AppDir
	cd build && $(MAKE) install DESTDIR=`pwd`/../AppDir
# Integrate verible
	wget "$(VERIBLE_URL)" \
		-O build/verible.tar.gz
	tar -xzf build/verible.tar.gz -C build/
	mv build/verible-${VERIBLE_VERSION}/bin/verible-verilog-syntax AppDir/usr/bin/verible-verilog-syntax
# Pack Final AppImage
	linuxdeploy --appdir AppDir/ --output appimage  --desktop-file src/appimage/picker.desktop --icon-file src/appimage/logo256.png

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
	rm -rf temp build dist picker_out* app_image_build AppDir

wheel: init
	cd dependence/xcomm && $(MAKE) wheel
	NO_BUILD_XSPCOMM=1 pipx run build
	mkdir -p dist && cp dependence/xcomm/dist/* ./dist

wheel_install: wheel
	pip3 uninstall -y xspcomm picker || true
	pip3 install dist/xspcomm*.whl dist/picker*.whl

# Run test suite
tests:
	$(MAKE) -C test all

smoke_tests:
	$(MAKE) -C test smoke

unit_tests:
	cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL=$(NPROC) $(ARGS)
	cd build && $(MAKE) -j$(NPROC)
	cd build && ctest --output-on-failure -j$(NPROC)
