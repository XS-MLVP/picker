
echo "clean up"
rm -rf build/rand

# build
bash script/run.sh example/hello/hello.v -t build/rand

# run
python3 build/rand/python/Hello.py
