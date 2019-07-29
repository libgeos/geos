#!/bin/sh

prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@/bin
libdir=@CMAKE_INSTALL_PREFIX@/lib

usage()
{
    cat <<EOF
Usage: geos-config [OPTIONS]
Options:
     [--prefix]
     [--version]
     [--libs]
     [--clibs]
     [--cclibs]
     [--static-clibs]
     [--static-cclibs]
     [--cflags]
     [--ldflags]
     [--includes]
     [--jtsport]
EOF
    exit $1
}

if test $# -eq 0; then
  usage 1 1>&2
fi

while test $# -gt 0; do
case "$1" in
    -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
    *) optarg= ;;
esac
case $1 in
    --prefix)
      echo @CMAKE_INSTALL_PREFIX@
      ;;
    --version)
      echo @GEOS_VERSION@
      ;;
    --cflags)
      echo -I@CMAKE_INSTALL_PREFIX@/include
      ;;
    --libs)
      echo -L@CMAKE_INSTALL_PREFIX@/lib -lgeos-@GEOS_VERSION_MAJOR@
      ;;
    --clibs)
      echo -L@CMAKE_INSTALL_PREFIX@/lib -lgeos_c
      ;;
    --cclibs)
      echo -L@CMAKE_INSTALL_PREFIX@/lib -lgeos
      ;;
    --static-clibs)
      echo -L@CMAKE_INSTALL_PREFIX@/lib -lgeos_c -lgeos -lm
      ;;
    --static-cclibs)
      echo -L@CMAKE_INSTALL_PREFIX@/lib -lgeos -lm
      ;;
    --ldflags)
      echo -L@CMAKE_INSTALL_PREFIX@/lib -lgeos
      ;;
    --includes)
      echo @CMAKE_INSTALL_PREFIX@/include
      ;;
    --jtsport)
    echo @JTS_PORT@
      ;;
    *)
      usage 1 1>&2
      ;;
  esac
  shift
done

