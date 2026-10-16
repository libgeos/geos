#!/bin/sh
. ../common.sh

if [ -n "$USE_GEOS_CONFIG" ]; then
  EXPECTED_LIBPATH="$(echo $(geos-config --ldflags) | sed 's/^-L//')"
else
  EXPECTED_LIBPATH="$(pkg-config geos --variable=libdir)"
fi

test_libpath c_app "${EXPECTED_LIBPATH}" libgeos_c
