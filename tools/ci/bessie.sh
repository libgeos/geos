#!/bin/sh
#
# FreeBSD 64 (bessie) CI script runner for GEOS
#
# Copyright (c) 2018 Regina Obe <lr@pcorp.us>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#

set -e

# auto tools
if false; then
    sh autogen.sh
    GEOS_INSTALL=/tmp/geos_autotools
    ./configure --prefix=$GEOS_INSTALL
    make
    make check
    make install
    ./tests/postinstall/test_geos-config.sh $GEOS_INSTALL
    ./tests/postinstall/test_pkg-config.sh $GEOS_INSTALL
fi


# cmake
if true; then
    rm -rf build
    mkdir -p build
    cd build
    GEOS_INSTALL=/tmp/geos_cmake
    cmake -DCMAKE_INSTALL_PREFIX=$GEOS_INSTALL ..
    make
    ctest --output-on-failure
    make install
    ./tests/postinstall/test_cmake.sh $GEOS_INSTALL
    ./tests/postinstall/test_pkg-config.sh $GEOS_INSTALL
    ./tests/postinstall/test_geos-config.sh $GEOS_INSTALL
fi
