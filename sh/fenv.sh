#echo "setting facets build env for $buildlayer"

# expect $buildlayer to be set to target version number by my .bashrc
  buildlayer=${buildlayer:-30}
  gsdir=$HOME/devel/f$buildlayer
  if [ $buildlayer = build ]; then
    export layer=/gcm/where/facets/30/sparc.Solaris
    buildlayer=30
  elif [ $buildlayer = here ]; then
    export layer=$HOME/devel/layers.f$buildlayer
    buildlayer=30
    gsdir=`pwd`
    
  else
    export layer=$HOME/devel/layers.f$buildlayer
  fi

  if [ -e /usr/bin/whoami ]; then
    export USER=`whoami`
  else
    export USER=`/usr/ucb/whoami`
  fi
  
# unset variables needed for full product builds
  unset GSJBASE
  unset ARCHBASE

cse="se/"
jtst=tests

if [ -d $layer/jdk12 -o $buildlayer = 41p1 ]; then
    jsrc=$layer/jdk12
    gsp=com/gemstone
elif [ -d $layer/jdk13 ]; then
    jsrc=$layer/jdk13
    gsp=com/gemstone
else
    jsrc=$layer/vm
    gsp=com/gemstone/persistence
    jca=$layer/jca/$gsp/connection/internal
fi
export jsrc


  SNAPSHOT_SOURCE=$gsdir
  SNAPSHOT=$SNAPSHOT_SOURCE/product

export GSJBASE=$layer

export GEMSTONE=$SNAPSHOT
#  echo "GEMSTONE=$GEMSTONE"

# make sure the snapshot has a key file
  if [ -d $GEMSTONE  -a  ! -r $GEMSTONE/sys/gemstone.key ]; then
        echo "Copying gemstone.key to $GEMSTONE/sys"
        cp $HOME/devel/keys/gsj50.key $GEMSTONE/sys/gemstone.key
  fi

# JTESTS is needed to run tests
export JTESTS=$layer/${jtst}

export ALT_BOOTDIR=/gcm/where/jdk/1.3.1.05/sparc.Solaris
#export USESUN=true

function go {
    case $1 in
        admin ) cd $layer/admin/$gsp/admin/internal ;;
        c )   cd $layer/orb/$gsp/CORBA ;;
        d )   cd $HOME/devel ;;
        dbf ) cd $HOME/devel/dbf${buildlayer} ;;
        ex )  cd $layer/tools/examples ;;
        g )   cd com/gemstone/persistence ;;
        i )   cd $layer/orb/$gsp/CORBA/se/internal ;;
        int ) cd $layer/tests/integration ;;
        it )  cd $layer/tools/$gsp/tools ;;
        jca ) cd $layer/jca ;;
        jcas ) cd $layer/tests/integration/jcaScale ;;
        jdo ) cd $layer/jdo/com/gemstone/persistence/jdo ;;
        ji ) cd $layer/jca/com/gemstone/persistence/connection/internal ;;
        jsrc ) cd $jsrc/jdk/src/share/classes ;;
        l )   cd $layer ;;
        log ) cd $layer/log10 ;;
        o )   cd $layer/orb ;;
        p )   cd $layer/orb/$gsp/PortableServer/internal ;;
        pe )  cd $layer/server/$gsp ;;
        smoke ) cd $JTESTS/smoke ;;
        t )   cd $JTESTS ;;
        to )  cd $layer/tools ;;
        tr )  cd $layer/transport/$gsp/transport ;;
        wp )  cd $JTESTS/integration/petstore/weblogic ;;
        wr )  cd $JTESTS/integration/run/weblogic ;;
        * )   cd ${1} ;;
    esac
}
  function gsjava {
    $GEMSTONE/bin/java $@
  }
  function gsjavac {
    $GEMSTONE/bin/javac $@
  }
  function makeprod {
    build/build.pl start=makeprod stop=makeprod $@
  }

if [ -d $GEMSTONE ]; then
    export JAVA_HOME=$GEMSTONE
    export PATH=$GEMSTONE/bin:$PATH
fi

unset jtst
unset neworb
unset gsdir
