#!/bin/sh
rm -f rerun.bt
props=`ls */*.prop | grep -v latest.prop`
for x in $props; do
  cat $x >>rerun.bt
done
emx rerun.bt
#hydra@ rerun.bt
