#!/bin/sh

# Post-install tests with pkg-config and a Makefile
#
# First required argument is the installed prefix, which
# is used to set PKG_CONFIG_PATH and rpath for shared.
# Second optional argument is either 'shared' (default) or 'static'
cd $(dirname $0)
. ./common.sh
main_setup $1 $2

_uname_s=$(uname -s)
echo "Running post-install tests on ${_uname_s} with pkg-config (${library_type})"

export PKG_CONFIG="${PKG_CONFIG:-pkg-config}"
export PKG_CONFIG_PATH=${prefix}/lib/pkgconfig

# "gmake" command (GNU Make) not always available
if ! command -v gmake >/dev/null 2>&1; then
  alias gmake=make
fi

if [ ${library_type} = shared ]; then
  export LDFLAGS="${LDFLAGS} -Wl,-rpath,$(pkg-config geos --variable=libdir)"
  case $_uname_s in
    MINGW64*)
      export PATH=$PATH:${prefix}/bin
      ;;
  esac
fi

make_all_test_clean(){
  set -e
  gmake -f pkgconfig.mak clean
  gmake -f pkgconfig.mak all
  gmake -f pkgconfig.mak test
  gmake -f pkgconfig.mak clean
}

echo "Testing C app"
cd c_app
make_all_test_clean
cd ..

echo "Testing C++ app"
cd cpp_app
make_all_test_clean
cd ..

echo "Finished running post-install tests with pkg-config (${library_type})"
