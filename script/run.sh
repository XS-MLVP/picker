cmake . -Bbuild
cd build
make
if [[ $? == 0 ]] ; then
  bin/mcv $@
fi
