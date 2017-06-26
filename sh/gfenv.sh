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
fi

if [ x"$USER" = x -a -e /usr/ucb/whoami ]; then
  export USER=`/usr/ucb/whoami`
fi

un=`uname`
#export GCMDIR=/export/ent1/users/bschuchardt/devel/gcm
export GCMDIR=/export/gcm

if [ $un = "Darwin" ]; then
  export JAVA_HOME=/Library/Java/Home
elif [ $un = "Linux" ]; then
  export JAVA_HOME=$GCMDIR/where/jdk/1.8.0_66/x86_64.linux
else
  export JAVA_HOME=$GCMDIR/where/jdk/1.8.0_45/sparc.Solaris
fi

export ANT_HOME=$GCMDIR/where/java/ant/apache-ant-1.8.4
PATH=$ANT_HOME/bin:$PATH

if [ -d $layer/open ]; then
  export CDPATH=$layer/closed:$layer/open:$CDPATH
fi

export PATH=$cmds:$JAVA_HOME/bin:$PATH

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

export GEMFIRE=$layer/closed/pivotalgf-assembly/build/install/pivotal-gemfire
PATH=$GEMFIRE/bin:$PATH

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
else
    ht=linux
fi

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
export SQLFIRE=$build/product-gfxd
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
if [ -r $SQLFIRE/quickstart ]; then
  SQLFCLASSPATH=$SQLFIRE/lib/gemfirexd.jar:$SQLFIRE/lib/gemfirexd-tools.jar:$SQLFIRE/lib/commons-cli-1.2.jar:$SQLFIRE/lib/jline-1.0.jar:$SQLFIRE/lib/jna-3.5.1.jar:$CLASSPATH
  PATH=$PATH:$SQLFIRE/bin
fi
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


