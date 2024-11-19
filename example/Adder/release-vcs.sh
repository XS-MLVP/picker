#!/bin/bash


if ! command -v verible-verilog-syntax &> /dev/null
then
    echo "verible could not be found"
    echo "please add verible-verilog-syntax into path first"
    echo "https://chipsalliance.github.io/verible/verilog_syntax.html"
    echo "https://github.com/chipsalliance/verible/releases/tag/v0.0-3428-gcfcbb82b"
    exit
fi

rm -rf picker_out_adder/
./build/bin/picker export example/Adder/Adder.v --autobuild false --sim vcs -w Adder.fsdb --sname Adder --tdir picker_out_adder/ --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/Adder/example.py picker_out_adder/Adder/python/
elif [[ $@ == *"java"* ]]; then
    cp example/Adder/example.java picker_out_adder/Adder/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/Adder/example.scala picker_out_adder/Adder/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/Adder/example.go picker_out_adder/Adder/golang/
else
    cp example/Adder/example.cpp picker_out_adder/Adder/cpp/
fi

# go to the Adder directory and make all
cd picker_out_adder/Adder && make EXAMPLE=ON

# go back to the root directory
cd ../

if [[ $@ != *"cpp"* ]]; then
    echo "'cannot allocate memory in static TLS block'error for VCS is expected, please ignore it"
fi

if [[ $@ == *"python"* ]]; then
    LD_PRELOAD=./Adder/libUTAdder.so python3 example.py
fi

if [[ $@ == *"scala"* ]]; then
    LD_PRELOAD=./Adder/libUTAdder.so scala -cp ./Adder/UT_Adder-scala.jar:./Adder/xspcomm-scala.jar -ea com.ut.example
fi

if [[ $@ == *"java"* ]]; then
    LD_PRELOAD=./Adder/libUTAdder.so java -cp ./Adder/UT_Adder-java.jar:./Adder/xspcomm-java.jar -ea com.ut.example
fi

if [[ $@ == *"golang"* ]]; then
    LD_PRELOAD=./Adder/golang/src/UT_Adder/libUTAdder.so GO111MODULE=off GOPATH="`pwd`/Adder/golang" go build example.go
    LD_PRELOAD=./Adder/golang/src/UT_Adder/libUTAdder.so GO111MODULE=off GOPATH="`pwd`/Adder/golang" ./example
fi
