
echo "--------------cleanup--------------"
rm -rf build/rand

# build
echo "--------------build--------------"
bash script/run.sh example/hello/hello.v -t build/rand

# run
echo "--------------run--------------"
python3 build/rand/python/Hello.py
