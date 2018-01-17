#!/bin/sh
cleanhydra
there=`pwd`
cd $layer
if [ -h pc.zip ]; then
  build/unzip
fi
build/quick
cd $there
hydra@
sleep 2
exec watchbt
