#!/bin/bash -e
#
# FreeBSD 64 (bessie) CI script runner for GEOS
#
# Copyright (c) 2018 Regina Obe <lr@pcorp.us>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#
sh autogen.sh
./configure
make
make check
