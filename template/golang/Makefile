CURRENT_DIR := $(shell pwd)

all: test clean
	@echo "All complete"

test: compile
	@echo "--------------Comple Success Test Step DUT--------------------"
	@cd ../ && GO111MODULE=off GOPATH="`pwd`/${TARGET}/golang:{{__XSPCOMM_GOLANG__}}" go run example.go && rm -rf *.fst *.vcd *.fsdb *.log *.key 2>/dev/null || true
	@echo "--------------------------------------------------------------"


# copy xspcomm lib
copy_xspcomm_lib={{__COPY_XSPCOMM_LIB__}}
ifneq ($(copy_xspcomm_lib), true)
copy_xspcomm:
	@echo "Ignore copy gomodule xspcomm"
else
copy_xspcomm:
	@echo "Try copy gomodule xspcomm"
	@cp -r {{__XSPCOMM_GOLANG__}} .
endif

compile: copy_xspcomm
	cmake . -Bbuild
	cmake --build build --config Debug --parallel `nproc`
	@mkdir -p golang/src/${TARGET}

	@mv build/${TARGET}.so golang/src/${TARGET}/
	@mv build/${TARGET}.go golang/src/${TARGET}/
	@mv *.so golang/src/${TARGET}/
	@mv *.so.* golang/src/${TARGET}/ || true
	@mv *.go golang/src/${TARGET}/
	@mv go.mod golang/src/${TARGET}/

clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key *.a build

purge: clean
	@ls|grep -v golang|xargs rm -rf
	@mv ./* ../ || true
	@echo "purge complete"
	@rm -rf $(CURRENT_DIR)
