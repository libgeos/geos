#!/bin/sh
#
# FreeBSD 32 (bessie32) CI script runner for GEOS
#
# Copyright (c) 2018 Regina Obe <lr@pcorp.us>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.

rm -rf build
mkdir -p build
cd build
cmake ../
make
[ -f CMakeCache.txt ] && \
ctest --output-on-failure . || \
make check
