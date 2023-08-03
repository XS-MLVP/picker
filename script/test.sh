cmake . -Bbuild
cd build
make
if [[ $? == 0 ]] ; then
  cd ..
  build/bin/$1 $@
fi
