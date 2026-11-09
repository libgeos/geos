#!/bin/sh
. ../common.sh

PROGRAM_VERSION=$(./c_app -v)

if [ -n "$USE_GEOS_CONFIG" ]; then
  EXPECTED_VERSION=$(geos-config --version)
else
  EXPECTED_VERSION=$(pkg-config geos --modversion)
fi
test_version ${PROGRAM_VERSION} ${EXPECTED_VERSION}
