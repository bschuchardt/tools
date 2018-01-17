#!/bin/sh
# getperf.sh analyses the results of smokeperf runs
#
# Each smokeperf run should be in its own directory.  The ones that
# you want to analyze and compare should be put in the "dirs" list
# below.
#
# Change the "gp" variable below to point to your checkout
#
# Change the "rpt" variable below to a different filename if you
# don't want results to go into "performance.txt"


#jan22_opengem is just before fastutils was introduced
#   8f96599cf9679a75af627cb42a09ac67f0989c75

#jan26_opengem 2016 is the fastutils change
#   2d275f2ff9ed59fca30466fc3f0160df6e661b0f

#dec31_opengem 2015
#   444d527172ae0b217aacf5230798f9cac5a62ae5

#nov28_opengem 2015
#   7b68c03e01f35b3138ab59bb46aa1b5ec549a74a

dirs="820X_rerun july2016"

#gp=$layer/closed/release/build/getperf.sh
gp=$layer/release/build/getperf.sh

rpt=performance.txt

if [ ! -r $gp ]; then
  echo "unable to locate the performance analysis tool - edit this script"
  echo "to point to your checkout"
  exit 1
fi

# now we run getperf.sh three times to generate ops/second, detailed and
# raw reports

$gp ops $dirs
mv perfcomparison.txt $rpt

$gp $dirs
cat perfcomparison.txt >>$rpt

$gp raw addTestKey $dirs
cat perfcomparison.txt >>$rpt

echo "results are in $rpt"

if [ x"$VISUAL" != x ]; then
  $VISUAL $rpt&
fi
