#!/bin/bash
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

dirs="gf98 devel notsettingsni"

function gfe
{
    buildlayer=${1:-trunk}
    . $HOME/.profile
    . $HOME/usr/sh/gfenv.sh
}

function getreport {
    addTestKey=""
    if [ $1 == "true" ]; then
      shift 1
      addTestKey="-DaddTestKey=true"
    fi
    mode=$1
    shift 1

    #statspec="-DstatSpecFile=${layer}/closed/gemfire-test/build/resources/test/cacheperf/specs/ops.spec"

    closeddir=${layer}/gemfire/closed
    classpath=${GEMFIRE}/lib/geode-dependencies.jar:\
${closeddir}/gemfire-test/build/classes/hydraClasses:\
${closeddir}/gemfire-test/build/resources/extraJars/groovy-all-2.4.3.jar

    $JAVA_HOME/bin/java -cp $classpath \
    -Xmx1024M \
    -DJTESTS=${closeddir}/gemfire-test/build/resources/test \
    -Dgemfire.home=${layer}/gemfire-assembly \
    -DrespectArgOrder=true \
    -DcompareByKey=true \
    -DomitFailedTests=true \
    -DaddTestKey=true \
    -Dmode=$mode \
    $statspec \
    $addTestKey perffmwk.PerfComparer $@
}


gfe dev

rpt=performance.txt

# now we run getperf.sh to generate ops/second and
# raw reports

rm -f perfcomparison.txt perfcomparer.log
getreport ratio $dirs
if [ ! -r perfcomparison.txt ]; then
  echo "something went wrong"
  exit 2
fi
mv perfcomparison.txt $rpt

getreport true raw $dirs
if [ ! -r perfcomparison.txt ]; then
  echo "something went wrong"
  exit 2
fi
cat perfcomparison.txt >>$rpt

echo "results are in $rpt"

if [ x"$VISUAL" != x ]; then
  $VISUAL $rpt&
fi
