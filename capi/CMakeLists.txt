#################################################################################
# $Id$
#
# GEOS C library build configuration for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################

set(geos_c_SOURCES
  geos_c.cpp
  geos_ts_c.cpp)

add_library(geos_c SHARED ${geos_c_SOURCES})

target_link_libraries(geos_c geos)