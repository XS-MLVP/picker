all: test clean
	@echo "All complete"

test: compile
	@echo "--------------Comple Success Test Step DUT--------------------"
	@cd ../ && GO111MODULE=off GOPATH="`pwd`/${TARGET}/golang" go run example.go || true
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
	@mv go.mod golang/src/${TARGET}/

clean:
	@rm -rf *.fst *.vcd *.fsdb *.log *.key *.a
