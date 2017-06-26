#! /bin/bash
exec 1>&1
exec 2>&1
exec 0>&1
set -v
set -x
date

if [ x"$GEMFIRE_BUILD" == x ]; then
  echo "GEMFIRE_BUILD isn't set!"
  exit 3
fi

#GEMFIRE_BUILD=/home/lynn/gemfireBuild/opengem_dev_Jan15
rm -r $GEMFIRE_BUILD

cd closed
#./gradlew clean -PbuildRoot=$GEMFIRE_BUILD/closed
./gradlew check --daemon -Dskip.tests=true -PbuildRoot=$GEMFIRE_BUILD/closed

cd open
#./gradlew clean -PbuildRoot=$GEMFIRE_BUILD/open
./gradlew check --daemon -Dskip.tests=true -PbuildRoot=$GEMFIRE_BUILD/open
