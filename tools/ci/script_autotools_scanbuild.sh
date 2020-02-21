#!/bin/sh
#
# Travis CI script for GEOS build with GNU Autotools
#
# Copyright (C) 2013 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#

if [ -z "${TRAVIS_BUILD_DIR+x}" ]; then
  echo TRAVIS_BUILD_DIR not defined
  exit 1
fi

# source common functions
. ${TRAVIS_BUILD_DIR}/tools/ci/common.sh

cd ${TRAVIS_BUILD_DIR}
./autogen.sh
cd -
scan-build ${TRAVIS_BUILD_DIR}/configure CC=clang-9 CXX=clang++-9 CXXFLAGS="-std=c++11"
scan-build --keep-cc --status-bugs make -j2
