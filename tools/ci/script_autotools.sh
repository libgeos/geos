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

set -e

if [ -z "${TRAVIS_BUILD_DIR+x}" ]; then
  echo TRAVIS_BUILD_DIR not defined
  exit 1
fi

# source common functions
. ${TRAVIS_BUILD_DIR}/tools/ci/common.sh

GEOS_INSTALL=/tmp/geos_autotools

cd ${TRAVIS_BUILD_DIR}
./autogen.sh
cd -
${TRAVIS_BUILD_DIR}/configure --prefix=${GEOS_INSTALL}
run_make
make check
make distcheck
make install
${TRAVIS_BUILD_DIR}/tests/postinstall/test_pkg-config.sh ${GEOS_INSTALL}
${TRAVIS_BUILD_DIR}/tests/postinstall/test_geos-config.sh ${GEOS_INSTALL}
