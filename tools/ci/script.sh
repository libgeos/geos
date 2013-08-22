#!/bin/bash -e
#
# Travis CI script runner for GEOS
#
# Copyright (c) 2013 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
source ${TRAVIS_BUILD_DIR}/tools/ci/common.sh

# prepare build directory
builddir="${TRAVIS_BUILD_DIR}/_build"
mkdir -p ${builddir}
cd ${builddir}

# build and run tests
${TRAVIS_BUILD_DIR}/tools/ci/script_${GEOS_TRAVIS_BUILD}.sh
