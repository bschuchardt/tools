#!/bin/bash
# this is part of a directory navigator for gemfire/enterprise

hn=`hostname`
if [ -d /${hn}2/users/bschuchardt/devel/testing ]; then
  tdir=/export/${hn}2/users/bschuchardt/devel/testing
elif [ -d /${hn}2/users/bschuchardt/devel/testing ]; then
  tdir=/export/${hn}2/users/bschuchardt/devel/testing
elif [ -d /${hn}1/users/bschuchardt/devel/testing ]; then
  tdir=/export/${hn}1/users/bschuchardt/devel/testing
elif [ -d /${hn}1/users/bschuchardt/devel/testing ]; then
  tdir=/export/${hn}1/users/bschuchardt/devel/testing
elif [ -d /${hn}a/users/bschuchardt/devel/testing ]; then
  tdir=/export/${hn}a/users/bschuchardt/devel/testing
else
  tdir=~/devel/testing
fi


if [ ! -r $layer/build.sh ]; then
open=$layer/open
coresrc=$open/geode-core/src/main/java
closed=$layer/closed
testsrc=$open/geode-core/src
utestsrc=$testsrc/test/java
case $1 in
    ai ) echo $coresrc/org/apache/geode/admin/internal ;;
    back ) echo $LPWD ;;  # set in the function 'go'
    bl ) echo $buildlayer ;;
    c ) echo $coresrc/org/apache/geode/cache ;;
    ci ) echo $coresrc/org/apache/geode/cache/client/internal ;;
    cl ) echo $closed ;;
    ct ) echo $closed/gemfire-test ;;
    core ) echo $open/geode-core ;;
    ct ) echo $closed/gemfire-test/src/test/java ;;
    d )   echo `dirname $layer` ;;
    di )  echo $coresrc/org/apache/geode/distributed/internal ;;
    dt )  echo ${tdir} ;;
    du )  echo $layer/open/geode-core/build/distributedTest ;;
    duo ) echo $layer/open/geode-core/build/test-results/binary/distributedTest ;;
    g )   echo $coresrc/org/apache/geode ;;
    gr )  echo $layer/open/geode-core/src/main/resources/org/apache/geode ;;
    hr )  echo $layer/closed/gemfire-test/src/test/resources ;;
    ht )  echo $layer/closed/gemfire-test/src/test/java ;;
    i )   echo $coresrc/org/apache/geode/internal ;;
    ic )  echo $coresrc/org/apache/geode/internal/cache ;;
    icp ) echo $coresrc/org/apache/geode/internal/cache/partitioned ;;
    it )  echo $coresrc/org/apache/geode/internal/tcp ;;
    l )   echo $layer ;;
    mi )  echo $coresrc/org/apache/geode/distributed/internal/membership/gms ;;
    o )   echo $layer/open ;;
    p )  echo $layer/open/geode-protobuf/src/main/java/org/apache/geode/protocol ;;
    r ) ls -tF | grep / | head -1 ;;
    rt ) dir=`ls -tF $tdir | grep / | head -1`; echo $tdir/$dir ;;
    smoke ) echo $layer/closed/pivotalgf-assembly/build/smokeTest ;;
    t )   echo $utestsrc ;;
    tg )  echo $utestsrc/org/apache/geode ;;
    tc )  echo $utestsrc/org/apache/geode/cache30 ;;
    tic ) echo $utestsrc/org/apache/geode/internal/cache ;;
    tr )  echo $layer/open/geode-core/src/test/resources/org/apache/geode ;;
    ts )  echo $coresrc/org/apache/geode/internal/cache/tier/sockets ;;
    tt )  echo $HOME/sdevel/caches/gfcache ;;
    * )   echo ${1} ;;
esac


else
case $1 in
    ai ) echo $layer/src/com/gemstone/gemfire/admin/internal ;;
    b ) echo $build ;;
    back ) echo $LPWD ;;  # set in the function 'go'
    bl ) echo $buildlayer ;;
    c ) echo $layer/src/com/gemstone/gemfire/cache ;;
    ci ) echo $layer/src/com/gemstone/gemfire/cache/client/internal ;;
    cu ) echo $layer/src/com/gemstone/gemfire/cache/util ;;
    d )   echo `dirname $layer` ;;
    dt )  echo ${tdir} ;;
    du )  ls -td $build/tests/results/dunit/dunit-tests-* | head -1 ;;
    dur ) ls -td $build/tests/results/dunit-rerun/dunit-tests-* | head -1 ;;
    cdu ) echo ./dunit/dunit-tests-* ;;
    ct )  echo /home/bschuchardt/sdevel/caches/tangosol ;;
    g )   echo $layer/src/com/gemstone/gemfire ;;
    i )   echo $layer/src/com/gemstone/gemfire/internal ;;
    ic )  echo $layer/src/com/gemstone/gemfire/internal/cache ;;
    icp ) echo $layer/src/com/gemstone/gemfire/internal/cache/partitioned ;;
    di )  echo $layer/src/com/gemstone/gemfire/distributed/internal ;;
    it )  echo $layer/src/com/gemstone/gemfire/internal/tcp ;;
    jgo )  echo $layer/src/com/gemstone/org/javagroups ;;
    jgcvs ) echo /export/ent1/users/bschuchardt/software/jgroups ;;
    jg ) echo $layer/src/com/gemstone/org/jgroups ;;
    jgs ) echo /export/ent1/users/bschuchardt/software/jgcvs/src/com/gemstone/org/jgroups ;;
    jgst ) echo /export/ent1/users/bschuchardt/software/jgcvs ;;
    jgt ) echo $HOME/sdevel/caches/jgperf ;;
    jsrc ) echo /home/bschuchardt/devel/layers.f30/vm/j2se/src/share/classes ;;
    l )   echo $layer ;;
    mi )  echo $layer/src/com/gemstone/gemfire/distributed/internal/membership/jgroup ;;
    p )   echo $jgdir/protocols ;;
    pb )  echo $jgdir/protocols/pbcast ;;
    r ) ls -tF | grep / | head -1 ;;
    rt ) dir=`ls -tF $tdir | grep / | head -1`; echo $tdir/$dir ;;
    s )   echo $GF_DEFAULT ;;
    t )   echo $layer/tests ;;
    tg )  echo $layer/tests/com/gemstone/gemfire ;;
    tc )  echo $layer/tests/com/gemstone/gemfire/cache30 ;;
    tic ) echo $layer/tests/com/gemstone/gemfire/internal/cache ;;
    tr )  echo $build/tests/results64 ;;
    ts )  echo $layer/src/com/gemstone/gemfire/internal/cache/tier/sockets ;;
    tt )  echo $HOME/sdevel/caches/gfcache ;;
    * )   echo ${1} ;;
esac
fi

