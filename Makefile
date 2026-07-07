SHELL := bash
.SHELLFLAGS := -eu -o pipefail -c

.PHONY: all init build install appimage test test_all test_vpi_all test_mem_direct_all \
        test_all_java test_all_scala clean wheel wheel_install tests smoke_tests unit_tests

include example/example.mk

export NPROC := $(shell (nproc 2>/dev/null || sysctl -n hw.ncpu) 2>/dev/null)
export BUILD_XSPCOMM_SWIG ?= python

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
# Pack Final AppImage
	linuxdeploy --appdir AppDir/ --output appimage  --desktop-file src/appimage/picker.desktop --icon-file src/appimage/logo256.png

test: build
	./build/bin/picker -h
	./build/bin/picker pack -h
	./build/bin/picker exports -h

test_all:
	rm -rf output
	@for example in $(EXAMPLE_TEST_ALL); do \
		$(MAKE) test_$$example EXAMPLE_LANG=python ARGS="$(ARGS)" || exit $$?; \
	done

test_vpi_all:
	@for lang in cpp golang java lua python scala; do \
		$(MAKE) test_InternalSignals EXAMPLE_LANG=$$lang ARGS="$(ARGS)" || exit $$?; \
	done

test_mem_direct_all:
	@for lang in cpp golang java lua python scala; do \
		$(MAKE) test_CacheSignalCFG EXAMPLE_LANG=$$lang ARGS="$(ARGS)" || exit $$?; \
	done

test_all_java:
	@for example in $(EXAMPLE_TEST_ALL_JAVA); do \
		$(MAKE) test_$$example EXAMPLE_LANG=java ARGS="$(ARGS)" || exit $$?; \
	done

test_all_scala:
	@for example in $(EXAMPLE_TEST_ALL_SCALA); do \
		$(MAKE) test_$$example EXAMPLE_LANG=scala ARGS="$(ARGS)" || exit $$?; \
	done

clean:
	rm -rf temp build dist output app_image_build AppDir picker_out* picker_e203_ifu_ift2icb

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
