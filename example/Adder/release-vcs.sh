#!/bin/bash

rm -rf picker_out/Adder
./build/bin/picker export example/Adder/Adder.v --autobuild false --sim vcs -w Adder.fsdb --sname Adder --tdir picker_out/Adder/ --sdir template $@
# if python in $@, then it will generate python binding
if [[ $@ == *"python"* ]]; then
    cp example/Adder/example.py picker_out/Adder/
elif [[ $@ == *"java"* ]]; then
    cp example/Adder/example.java picker_out/Adder/Adder/java/
elif [[ $@ == *"scala"* ]]; then
    cp example/Adder/example.scala picker_out/Adder/Adder/scala/
elif [[ $@ == *"golang"* ]]; then
    cp example/Adder/example.go picker_out/Adder/Adder/golang/
else
    cp example/Adder/example.cpp picker_out/Adder/Adder/cpp/
fi

# go to the Adder directory and make all
cd picker_out/Adder/Adder && make EXAMPLE=ON

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
