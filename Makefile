export BUILD_XSPCOMM_SWIG?=python

all: clean init build

init:
	rm -rf dependence/xcomm
	git clone https://github.com/XS-MLVP/xcomm.git dependence/xcomm 
	# checkout the same branch as the parent project
	@PARENT_BRANCH=`git branch --show-current` && cd dependence/xcomm && git checkout $$PARENT_BRANCH || echo "there is no branch $$PARENT_BRANCH in xcomm"
	
build:
	@if ! command -v verible-verilog-format ; then \
		echo "verible-verilog-format could not be found, please install verible first"; \
		echo "you can install verible by following command:"; \
		echo "wget \"https://github.com/chipsalliance/verible/releases/download/v0.0-4007-g98bdb38a/verible-v0.0-4007-g98bdb38a-linux-static-$(shell uname -m).tar.gz\""; \
		echo "tar -xzf verible-v0.0-4007-g98bdb38a-linux-static-$(uname -m).tar.gz"; \
		echo "mv verible-v0.0-4007-g98bdb38a/verible-verilog-format /usr/local/bin/"; \
		echo "or you can install in user local directory, remember to add ~/.local/bin to your PATH"; \
		echo "mv verible-v0.0-4007-g98bdb38a/verible-verilog-format ~/.local/bin/"; \
		exit 1; \
	fi
	cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL=`nproc` $(ARGS)
	cd build && make -j`nproc`

install: build
	cd build && make install

appimage:
	rm -rf AppDir app_image_build
# Build Picker Binary and XSPCOMM library
	cmake -DCMAKE_INSTALL_PREFIX=/usr . -Bapp_image_build -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_PARALLEL=`nproc` $(ARGS) 
	cd app_image_build && make -j`nproc` && make install DESTDIR=`pwd`/../AppDir
# Intergrate verible
	wget "https://github.com/chipsalliance/verible/releases/download/v0.0-4007-g98bdb38a/verible-v0.0-4007-g98bdb38a-linux-static-$(shell uname -m).tar.gz" \
		-O app_image_build/verible.tar.gz
	tar -xzf app_image_build/verible.tar.gz -C app_image_build/
	mv app_image_build/verible-v0.0-4007-g98bdb38a/bin/verible-verilog-syntax AppDir/usr/bin/verible-verilog-syntax
# Packing Final AppImage
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
	rm -rf temp build dist picker_out* app_image_build AppDir dependence/xcomm

wheel: clean
	cd dependence/xcomm && make wheel
	NO_BUILD_XSPCOMM=1 pipx run build
	cp dependence/xcomm/dist/* ./dist

wheel_install: wheel
	pip uninstall -y xspcomm picker
	pip install dist/xspcomm*.whl dist/picker*.whl