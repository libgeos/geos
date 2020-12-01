#!/bin/sh

# Post-install tests with CMake
#
# First required argument is the installed prefix, which
# is used to set CMAKE_PREFIX_PATH
set -e

echo "Running post-install tests with CMake"

CMAKE_PREFIX_PATH=$1
if [ -z "$CMAKE_PREFIX_PATH" ]; then
    echo "First positional argument CMAKE_PREFIX_PATH required"
    exit 1
fi

echo "CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH"

cd $(dirname $0)

cd test_c
rm -rf build
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH ..
VERBOSE=1 make
ctest --output-on-failure
cd ..
rm -rf build
cd ..

cd test_cpp
rm -rf build
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH ..
VERBOSE=1 make
ctest --output-on-failure
cd ..
rm -rf build

cd ..
