#!/bin/bash
passed=`grep -il "have a pleasant day" */Master*.log`
if [ x"$passed" == x ]; then
  exit 0
fi
for x in $passed; do
  directory=`dirname $x`
  if [ ! -h $directory ]; then
    echo "rm -rf $directory"
    rm -rf $directory
  fi
done
