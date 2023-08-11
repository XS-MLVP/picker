# build

echo "--------------cleanup--------------"
rm -rf build/rand

echo "--------------build secret--------------"

# flags
VERILATOR_FLAGS=" -cc -x-assign fast -Wall -CFLAGS -DVL_TIME_CONTEXT"
TOP_VERILATOR_FLAGS="${VERILATOR_FLAGS} --trace"

verilator ${VERILATOR_FLAGS} --protect-lib verilated_secret -Mdir build/rand example/protect_lib/secret_impl.v
make -j 4 -C build/rand -f Vsecret_impl.mk

echo "--------------build python--------------"
bash script/run.sh example/protect_lib/top.v -e build/rand/verilated_secret.sv -v --trace,-LDFLAGS,libverilated_secret.a -t build/rand -n protect -c -DVL_DEBUG=1 -d


echo "--------------run--------------"
python3 example/protect_lib/tb.py build/rand/python +trace
