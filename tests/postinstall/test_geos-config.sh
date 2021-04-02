#!/bin/sh

# Post-install tests with geos-config
#
# First required argument is the installed prefix, which
# is used to set PATH and LD_LIBRARY_PATH/DYLD_LIBRARY_PATH

echo "Running post-install tests with geos-config"

prefix=$1
if [ -z "${prefix}" ]; then
    echo "First positional argument to the the installed prefix is required"
    exit 1
fi

UNAME=$(uname)
case ${UNAME} in
  Darwin*)
    alias ldd="otool -L"
    export DYLD_LIBRARY_PATH=${prefix}/lib
    ;;
  Linux*)
    export LD_LIBRARY_PATH=${prefix}/lib
    ;;
  MINGW* | MSYS*)
    prefix=$(cygpath -u ${prefix})
    export LD_LIBRARY_PATH=${prefix}/bin
    ;;
  *)
    echo "no ldd equivalent found for UNAME=${UNAME}"
    exit 1 ;;
esac

export PATH=${prefix}/bin:${PATH}

# Prepare expected test outputs
export EXPECTED_GEOS_VERSION=$(geos-config --version)
export EXPECTED_LDD_SUBSTR=${prefix}/lib/libgeos
ERRORS=0

cd $(dirname $0)

. ./common.sh

echo "Testing C app"
cd test_c
make -f geos-config.mak clean
make -f geos-config.mak all

# run tests
check_ldd test_c
if [ $? -ne 0 ]; then ERRORS=$((ERRORS+1)); fi
check_version test_c
if [ $? -ne 0 ]; then ERRORS=$((ERRORS+1)); fi

make -f geos-config.mak clean

cd ..

echo "Testing C++ app"
cd test_cpp

make -f geos-config.mak clean
make -f geos-config.mak all

# run tests
check_ldd test_cpp
if [ $? -ne 0 ]; then ERRORS=$((ERRORS+1)); fi
check_version test_cpp
if [ $? -ne 0 ]; then ERRORS=$((ERRORS+1)); fi

make -f geos-config.mak clean

cd ..

echo "$0: ${ERRORS} tests failed"
exit ${ERRORS}
