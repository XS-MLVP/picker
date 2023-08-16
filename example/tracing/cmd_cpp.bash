echo "--------------cleanup--------------"
rm -rf build/rand

# build
echo "--------------build--------------"
bash script/run.sh example/tracing/top.v -t build/rand -l cpp -v example/tracing/vflags.txt -n trace -c -DVL_DEBUG=1
rm -f build/rand/cpp/Test_Trace.cpp
cp example/tracing/tb.cpp build/rand/cpp
cd build/rand/cpp
make

# run
echo "--------------run--------------"
LD_LIBRARY_PATH=. time ./Trace +trace
