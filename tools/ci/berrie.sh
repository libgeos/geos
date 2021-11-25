#!/bin/sh
#
# Raspberry Pi (berrie) 32-bit ARM CI script runner for GEOS
#
# Copyright (c) 2018-2021 Regina Obe <lr@pcorp.us>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.

# read version from Version.txt file
. Version.txt
echo ~/workspace/geos
export REL_PATH=~/workspace/geos/rel-${GEOS_VERSION_MAJOR}.${GEOS_VERSION_MINOR}.${GEOS_VERSION_PATCH}
rm -rf build
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=${REL_PATH} ../
make && make install
[ -f CMakeCache.txt ] && \
ctest --output-on-failure . || \
make check
