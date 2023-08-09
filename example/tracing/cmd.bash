echo "clean up"
rm -rf build/rand

# build
bash script/run.sh example/tracing/top.v -t build/rand -v example/tracing/vflags.txt -n trace -c -DVL_DEBUG=1

# run
python3 example/tracing/tb.py build/rand/python +trace