#!/bin/sh -e

# Post-install tests with CMake
#
# First required argument is the installed prefix, which
# is used to set CMAKE_PREFIX_PATH
# Second optional argument is either 'shared' (default) or 'static'
cd $(dirname $0)
. ./common.sh
main_setup $1 $2

# CMake command options options differ since CMake 4.4
CMAKE_MAJOR_MINOR=$(cmake --version | grep -o '[[:digit:]]\+\.[[:digit:]]\+')
cmp_44=$(printf "4.4\n${CMAKE_MAJOR_MINOR}\n")
sorted_44=$(echo "$cmp_44" | sort -V)
if [ "$cmp_44" = "$sorted_44" ]; then  # CMake 4.4 or later
    CMAKE_OPTIONS="-Werror=author -Wno-error=deprecated --log-level=VERBOSE"
else  # Before CMake 4.4 - no way to suppress "deprecated", so just warn
    CMAKE_OPTIONS="-Wdev --log-level=VERBOSE"
fi

_uname_s=$(uname -s)
echo "Running post-install tests on ${_uname_s} with CMake (${library_type})"

if [ "x${library_type}" = xshared ]; then
  case $_uname_s in
    MINGW64*)
      export PATH=$(cygpath -u ${prefix})/bin:$PATH
      ;;
  esac
fi

cmake_make_ctest(){
  rm -rf build

  cmake ${CMAKE_OPTIONS} \
    -D CMAKE_PREFIX_PATH=${prefix} \
    -D CMAKE_COMPILE_WARNING_AS_ERROR=ON \
    -S . -B build

  cmake --build build --verbose

  ctest --test-dir build --output-on-failure --verbose

  rm -rf build
}

echo "Testing C app"
cd c_app
cmake_make_ctest
cd ..

echo "Testing C++ app"
cd cpp_app
cmake_make_ctest
cd ..

echo "Finished running post-install tests CMake (${library_type})"
