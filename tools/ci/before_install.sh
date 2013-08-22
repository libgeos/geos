#!/bin/bash -e
#
# Travis CI before_install runner for GEOS
#
# Copyright (c) 2013 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
source ${TRAVIS_BUILD_DIR}/tools/ci/common.sh

sudo apt-get update -qq

before_install="${TRAVIS_BUILD_DIR}/tools/ci/before_install_${GEOS_TRAVIS_BUILD}.sh"
[ -x ${before_install} ] && ${before_install} || echo "nothing to run"

