#!/bin/sh
exec 2>&1
exec >stresstest.log
for x in "$@"; do
  stressTest="$stressTest --test $x"
done
gr repeatDistributedTest $stressTest \
  --no-parallel -Prepeat=50 -PfailOnNoMatchingTests=false
