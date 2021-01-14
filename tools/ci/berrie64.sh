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

rm -rf build
mkdir -p build
cd build
cmake ../
make
[ -f CMakeCache.txt ] && \
ctest --output-on-failure . || \
make check
