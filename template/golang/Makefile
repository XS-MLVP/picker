
CGO_LIB_XSPCOMM=`pwd`/${TARGET}/golang/src/xspcomm/golangxspcomm.so
CGO_LIB_UT=`pwd`/${TARGET}/golang/src/${TARGET}/${TARGET}.so

all: test clean
	@echo "All complete"

test: compile
	@echo "--------------Comple Success Test Step DUT--------------------"
	@cd ../ && CGO_LDFLAGS="${CGO_LIB_XSPCOMM} ${CGO_LIB_UT}" GO111MODULE=off GOPATH="`pwd`/${TARGET}/golang" go run example.go || true
	@echo "--------------------------------------------------------------"

compile:
	@cp -r {{__XSPCOMM_GOLANG__}} .
	cmake . -Bbuild
	cmake --build build --config Debug --parallel `nproc`
	@mkdir -p golang/src/${TARGET}

	@mv build/${TARGET}.so golang/src/${TARGET}/
	@mv build/${TARGET}.go golang/src/${TARGET}/
	@mv *.so golang/src/${TARGET}/
	@mv *.go golang/src/${TARGET}/

clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key
