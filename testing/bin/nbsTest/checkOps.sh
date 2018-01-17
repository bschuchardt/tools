#!/bin/sh
# args should be
# Create/Destroy/Invalidate/Update
op="after${1:-Create}"
reps="8"
#if [ $op = afterCreate ]; then
#  reps="4"
#fi
fname="${op}.txt"
set -x
set -v
grep ${op} vm*client*.log vm*Client*.log | sort --key=15 >${fname}
java -cp .. checkOps ${fname} ${op} ${reps}
