#!/bin/sh
#
# Raspberry Pi (berrie64) CI script runner for GEOS
# 64-bit ARM chip
#
# Copyright (c) 2020 Regina Obe <lr@pcorp.us>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.

set -e

rm -rf build
mkdir -p build
cd build
GEOS_INSTALL=/tmp/geos_cmake
cmake -DCMAKE_INSTALL_PREFIX=${GEOS_INSTALL} ..
make
ctest --output-on-failure
make install
../tests/postinstall/test_cmake.sh ${GEOS_INSTALL}
../tests/postinstall/test_pkg-config.sh ${GEOS_INSTALL}
../tests/postinstall/test_geos-config.sh ${GEOS_INSTALL}
