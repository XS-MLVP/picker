all: clean build

init:
	rm -rf dependence/xcomm
	git clone https://github.com/XS-MLVP/xcomm.git dependence/xcomm
	
build:
	@if ! command -v verible-verilog-format ; then \
		echo "verible-verilog-format could not be found"; \
		echo "install verible-verilog-format"; \
    	echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"; \
		exit 1; \
	fi
	cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release
	cd build && make -j`nproc`

install: build
	cd build && make install

clean:
	sudo rm -rf temp build