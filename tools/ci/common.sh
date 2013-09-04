#!/bin/bash -e
#
# Common utilities for Travis CI setup for GEOS
#
# Copyright (c) 2013 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
if [[ "$TRAVIS" != "true" ]] ; then
	echo "Running this script makes no sense outside of travis-ci.org"
	exit 1
fi
#
# Environment
#
TCI_NUMTHREADS=2
if [[ -f /sys/devices/system/cpu/online ]]; then
	# Calculates 1.5 times physical threads
	TCI_NUMTHREADS=$(( ( $(cut -f 2 -d '-' /sys/devices/system/cpu/online) + 1 ) * 15 / 10  ))
fi
#
# Functions
#
tmstamp()
{
    echo -n "[$(date '+%H:%M:%S')]" ;
}

run_make()
{
    [ $TCI_NUMTHREADS -gt 0 ] && make -j $TCI_NUMTHREADS || make 
}

run_test()
{
    [ -f CMakeCache.txt ] && \
        ctest -V --output-on-failure . || \
        make check
}
