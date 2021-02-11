#!/bin/sh
#
# Debian sid CI script runner for GEOS
# 64-bit Debian Sid (debbie)
#
# Copyright (c) 2021 Regina Obe <lr@pcorp.us>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
export REL_PATH=${WORKSPACE}/rel-${GEOS_VER}w${OS_BUILD}
rm -rf build
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=${REL_PATH} ../
make && make install
[ -f CMakeCache.txt ] && \
ctest --output-on-failure . || \
make check
