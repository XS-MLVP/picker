cmake . -Bbuild
cd build
make -j`nproc`
if [[ $? == 0 ]] ; then
  cd ..
  build/bin/mcv $@
fi
