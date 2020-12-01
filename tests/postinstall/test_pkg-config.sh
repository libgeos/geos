#!/bin/sh

# Post-install tests with pkg-config
#
# First required argument is the installed prefix, which
# is used to set PKG_CONFIG_PATH and LD_LIBRARY_PATH/DYLD_LIBRARY_PATH

echo "Running post-install tests with pkg-config"

prefix=$1
if [ -z "$prefix" ]; then
    echo "First positional argument to the the installed prefix is required"
    exit 1
fi

export PKG_CONFIG_PATH=$prefix/lib/pkgconfig

UNAME=$(uname)
case $UNAME in
  Darwin*)
    alias ldd="otool -L"
    export DYLD_LIBRARY_PATH=$prefix/lib
    ;;
  Linux*)
    export LD_LIBRARY_PATH=$prefix/lib
    ;;
  *)
    echo "no ldd equivalent found for UNAME=$UNAME"
    exit 1 ;;
esac

# Prepare expected test outputs
export EXPECTED_GEOS_VERSION=$(pkg-config geos --modversion)
export EXPECTED_LDD_SUBSTR=$prefix/lib/libgeos

cd $(dirname $0)

echo "Testing C app (only)"
cd test_c

aclocal
automake --add-missing --copy --foreign
autoconf
./configure
exit=$?
if [ $exit -ne 0 ]; then
  exit $exit
fi
make clean
make
make check
exit=$?
if [ $exit -ne 0 ]; then
  ls
  cat test-suite.log
  exit $exit
fi
make clean
