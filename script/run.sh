cmake . -Bbuild
cd build
make
if [[ $? == 0 ]] ; then
  cd ..
  build/bin/mcv $@
fi
