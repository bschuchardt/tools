#set -v
#set -x

#echo "setting gemfire build env for $buildlayer"

# expect $buildlayer to be set to target version number by my .bashrc
buildnum=${buildlayer:-trunk}
hn=`hostname`
hn=`echo $hn | cut -d . -f 1`
if [ $buildnum = "at" ]; then
    shift 1
    export layer=$1
elif [ $buildnum = "here" ]; then
    export layer=$PWD
elif [ -d /Applications ]; then
    export layer=/Users/bschuchardt/devel/gf$buildnum
elif [ -d /${hn}2/users/bschuchardt/devel/gf$buildnum ]; then
    export layer=/export/${hn}2/users/bschuchardt/devel/gf$buildnum
elif [ -d /${hn}2/users/bschuchardt/devel/gf$buildnum ]; then
    export layer=/export/${hn}2/users/bschuchardt/devel/gf$buildnum
elif [ -d /${hn}1/users/bschuchardt/devel/gf$buildnum ]; then
    export layer=/export/${hn}1/users/bschuchardt/devel/gf$buildnum
elif [ -d /${hn}1/users/bschuchardt/devel/gf$buildnum ]; then
    export layer=/export/${hn}1/users/bschuchardt/devel/gf$buildnum
#elif [ -d /export/shared_build/users/bschuchardt/gf$buildnum ]; then
#    export layer=/export/shared_build/users/bschuchardt/gf$buildnum

#elif [ $hn == "w1-gst-dev01" ]; then
#    export layer=/export/w1-gst-dev01d/users/bschuchardt/devel/gf$buildnum
else
    export layer=/home/bschuchardt/devel/gf$buildnum
fi
if [ ! -d $layer ]; then
  echo "warning: directory doesn't exist '$layer'"
else
  echo "build root is $layer"
  if [ -r $layer/gemfire-assembly ]; then
    closedDir=$layer/gemfire/closed
  else
    closedDir=$layer/closed
  fi
  export closedDir
fi

if [ x"$USER" = x -a -e /usr/ucb/whoami ]; then
  export USER=`/usr/ucb/whoami`
fi

un=`uname`
#export GCMDIR=/export/ent1/users/bschuchardt/devel/gcm
export GCMDIR=/export/gcm

if [ $un = "Darwin" ]; then
  export JAVA_HOME=/Library/Java/JavaVirtualMachines/jdk1.8.0_171.jdk/Contents/Home
elif [ $un = "Linux" ]; then
  export JAVA_HOME=/gcm/where/jdk/1.8.0_181/x86_64.linux
else
  export JAVA_HOME=$GCMDIR/where/jdk/1.8.0_45/sparc.Solaris
fi


#if [ -d $layer/open ]; then
#  export CDPATH=$CDPATH:$layer/closed:$layer/open
#fi

export PATH=$cmds:$JAVA_HOME/bin:$PATH
export JTESTS=${layer}/closed/gemfire-test/build/resources/test

function go {
  while [ x"$1" != x ]; do
    dir=`go.sh $1`
    export LPWD=$PWD
  #  if [ ! -d $dir ]; then
  #    echo "$dir - not a directory"
  #  else
      echo $dir
      cd $dir
  #  fi
  shift 1
  done
}

export go

if [ ! -r $layer/build.sh ]; then

if [ $un != "Darwin" ]; then
  export GRADLE_USER_HOME=$HOME/.grhomes/`hostname`
  #if [ ! -d $GRADLE_USER_HOME ]; then
  #  echo "gradle user home not found:  use mkdir \$GRADLE_USER_HOME"
  #fi
fi

export ocmds=~/usr/opensh
PATH=$ocmds:$PATH

if [ -r $layer/gemfire-assembly ]; then
  GEMFIRE=$layer/gemfire-assembly/build/install/pivotal-gemfire/apache-geode*
else
  GEMFIRE=$closedDir/pivotalgf-assembly/build/install/pivotal-gemfire
fi
PATH=$PATH:$GEMFIRE/bin

CLASSPATH=$GEMFIRE/lib/geode-dependencies.jar:$CLASSPATH

export gdir=$layer/open/geode-core/src/main/java/org/apache/geode
export intdir=$gdir/internal
export icdir=$gdir/internal/cache
export didir=$gdir/distributed/internal
export tgdir=$layer/closed/gemfire-test/src/main/java/org/apache/geode
export tsdir=$intdir/cache/tier/sockets
export ttsdir=$tgdir/internal/cache/tier/sockets



else # if open-source

un=`uname`
if [ $un = SunOS ]; then
    ht=sol
elif [ $un = AIX ]; then
    ht=aix
elif [ $un = Darwin ]; then
    ht=mac
else
    ht=linux
fi

export ANT_HOME=$GCMDIR/where/java/ant/apache-ant-1.8.4
PATH=$ANT_HOME/bin:$PATH

unset buildlayer
if [ -r $layer/build$ht.properties ]; then
  buildlayer=`grep "^osbuild.dir" $layer/build$ht.properties`
  if [ x"$buildlayer" != x ]; then
    buildlayer=`echo $buildlayer | cut -d "=" -f 2 -`
    buildlayer=`dirname $buildlayer`
    buildlayer=`dirname $buildlayer`
  fi
fi
if [ x"$buildlayer" = x ]; then
  buildlayer=$layer
fi

#echo buildlayer=$buildlayer
export buildlayer
export build=$buildlayer/build-artifacts/$ht
export GEMFIRE=$build/product
export JTESTS=$build/tests/classes

if [ x"$LD_LIBRARY_PATH" != x ]; then
  export LD_LIBRARY_PATH=$GEMFIRE/lib:$LD_LIBRARY_PATH
else
  export LD_LIBRARY_PATH=$GEMFIRE/lib
fi
export CLASSPATH=.:$GEMFIRE/lib/gemfire.jar:$HOME/usr/classes:\
$build/tests/classes:$GEMFIRE/lib/gemfire-core-dependencies.jar:\
$JTESTS
GFCLASSPATH=$CLASSPATH
export PATH=$GEMFIRE/bin:$PATH
export gdir=$layer/src/com/gemstone/gemfire
export intdir=$gdir/internal
export icdir=$gdir/internal/cache
export didir=$gdir/distributed/internal
export jgdir=$layer/src/com/gemstone/org/jgroups
export pdir=$jgdir/protocols
export jgmm=$gdir/distributed/internal/membership/jgroup/JGroupMembershipManager.java
export tgdir=$layer/tests/com/gemstone/gemfire
export tsdir=$intdir/cache/tier/sockets
export ttsdir=$tgdir/internal/cache/tier/sockets

branches=https://svn.gemstone.com/repos/gemfire/branches

fi # if [-d $layer/open ]


