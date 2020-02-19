#!/bin/sh

prefix=@ESCAPED_INSTALL_PREFIX@
libdir=${prefix}/lib

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
      echo ${prefix}
      ;;
    --version)
      echo @GEOS_VERSION@
      ;;
    --libs)
      # TODO: make an alias for --clibs
      # see http://trac.osgeo.org/geos/ticket/497
      echo -L${libdir} -lgeos-@GEOS_VERSION_MAJOR@
      ;;
    --clibs)
      echo -L${libdir} -lgeos_c
      ;;
    --cclibs)
      echo -L${libdir} -lgeos
      ;;
    --static-clibs)
      echo -L${libdir} -lgeos_c -lgeos -lm
      ;;
    --static-cclibs)
      echo -L${libdir} -lgeos -lm
      ;;
    --cflags)
      echo -I${prefix}/include
      ;;
    --ldflags)
      echo -L${libdir}
      ;;
    --includes)
      echo ${prefix}/include
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

