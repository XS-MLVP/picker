SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

.PHONY: all init build install appimage test test_all test_vpi_all test_mem_direct_all \
        test_all_java test_all_scala clean wheel wheel_install tests smoke_tests unit_tests

export BUILD_XSPCOMM_SWIG ?= python
verible_arch := $(shell uname -m)
ifneq ($(verible_arch),x86_64)
	verible_arch := $(shell echo $(verible_arch) | sed 's/aarch64/arm64/')
endif

# Default target should not destroy previous builds
all: init build

init:
	@if ! command -v verible-verilog-syntax >/dev/null 2>&1; then \
		echo "verible-verilog-syntax could not be found, please install verible first"; \
		echo "you can install verible by following command:"; \
		echo "\t$$ wget \"https://github.com/chipsalliance/verible/releases/download/v0.0-4007-g98bdb38a/verible-v0.0-4007-g98bdb38a-linux-static-${verible_arch}.tar.gz\""; \
		echo "\t$$ tar -xzf verible-v0.0-4007-g98bdb38a-linux-static-${verible_arch}.tar.gz"; \
		echo "\t$$ mv verible-v0.0-4007-g98bdb38a/bin/verible-verilog-format /usr/local/bin/"; \
		echo "or you can install in user local directory, remember to add ~/.local/bin to your PATH"; \
		echo "\t$$ mv verible-v0.0-4007-g98bdb38a/bin/verible-verilog-format ~/.local/bin/"; \
		exit 1; \
	fi
	@if [ ! -d dependence/xcomm/.git ]; then \
		mkdir -p dependence && \
		git clone --depth=1 https://github.com/XS-MLVP/xcomm.git dependence/xcomm; \
	else \
		git -C dependence/xcomm fetch --all --tags --prune; \
	fi
	# Checkout same branch as parent if it exists; otherwise fallback to latest master
	@PARENT_BRANCH=$$(git branch --show-current); \
	if git -C dependence/xcomm ls-remote --exit-code --heads origin "$$PARENT_BRANCH" >/dev/null 2>&1; then \
		echo "[xcomm] Using branch $$PARENT_BRANCH"; \
		git -C dependence/xcomm checkout -B "$$PARENT_BRANCH" "origin/$$PARENT_BRANCH"; \
	else \
		echo "[xcomm] No branch $$PARENT_BRANCH; fallback to latest master"; \
		git -C dependence/xcomm fetch origin master; \
		git -C dependence/xcomm checkout -B master origin/master; \
	fi
	
build:
	cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL=`nproc` $(ARGS)
	cd build && $(MAKE) -j`nproc`

clean_build:
	rm -rf build

rebuild: clean_build build

install: build
	cd build && $(MAKE) install

VERIBLE_VERSION ?= v0.0-4007-g98bdb38a

appimage: init
	rm -rf AppDir
# Reuse existing build if present; otherwise configure and build
	@if [ ! -d build ]; then \
		cmake -DCMAKE_INSTALL_PREFIX=/usr . -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL=`nproc` $(ARGS); \
		cd build && $(MAKE) -j`nproc`; \
	else \
		echo "Using existing build directory for AppImage"; \
	fi
# Install into AppDir
	cd build && $(MAKE) install DESTDIR=`pwd`/../AppDir
# Integrate verible
	wget "https://github.com/chipsalliance/verible/releases/download/${VERIBLE_VERSION}/verible-${VERIBLE_VERSION}-linux-static-${verible_arch}.tar.gz" \
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
	cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL=`nproc` $(ARGS)
	cd build && ctest --output-on-failure -j`nproc`
