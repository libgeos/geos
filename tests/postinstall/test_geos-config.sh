#!/bin/sh

# Post-install tests with geos-config and a Makefile
#
# First required argument is the installed prefix, which
# is used to set PATH to find geos-config and rpath for shared.
# Second optional argument is either 'shared' (default) or 'static'
cd $(dirname $0)
. ./common.sh
main_setup $1 $2

_uname_s=$(uname -s)
echo "Running post-install tests on ${_uname_s} with geos-config (${library_type})"

export USE_GEOS_CONFIG=1
export PATH="${prefix}/bin:${PATH}"

# "gmake" command (GNU Make) not always available
if ! command -v gmake >/dev/null 2>&1; then
  alias gmake=make
fi

if [ ${library_type} = shared ]; then
  _ldflags=$(echo $(geos-config --ldflags) | sed 's/^-L//')
  export LDFLAGS="${LDFLAGS} -Wl,-rpath,${_ldflags}"
fi

make_all_test_clean(){
  set -e
  gmake -f geosconfig.mak clean
  gmake -f geosconfig.mak all
  gmake -f geosconfig.mak test
  gmake -f geosconfig.mak clean
}

echo "Testing C app"
cd c_app
make_all_test_clean
cd ..

echo "Testing C++ app"
cd cpp_app
make_all_test_clean
cd ..

echo "Finished running post-install tests with geos-config (${library_type})"
