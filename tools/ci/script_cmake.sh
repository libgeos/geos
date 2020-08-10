#!/bin/sh
#
# Travis CI script for GEOS build with CMake
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

# return on first failure
set -e

cmake --version

cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_DOCUMENTATION=YES ${TRAVIS_BUILD_DIR}
run_make
cmake --build . --target docs
ctest --output-on-failure .

if [ "${BUILD_TYPE}" = "Coverage" ]; then
    curl -o codecov.sh https://codecov.io/bash
    bash codecov.sh
fi
