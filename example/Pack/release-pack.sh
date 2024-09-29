rm -rf pack_example
mkdir pack_example && cd pack_example

cp ../example/Pack/adder_trans.sv .
picker pack adder_trans -e

if [ "$1" = "both" ]; then
    cd adder_trans/example && make 

elif [ "$1" = "send" ]; then
    cp -r ../example/Pack/UVM2Python . && cd UVM2Python
    cp ../adder_trans/example/Makefile .
    make
elif [ "$1" = "receive" ]; then
    cp -r ../example/Pack/Python2UVM . && cd Python2UVM
    cp ../adder_trans/example/Makefile .
    make
fi