################################################################################
# Part of CMake configuration for GEOS
#
# Copyright (C) 2018 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
################################################################################
file(GLOB_RECURSE _sources ${CMAKE_CURRENT_LIST_DIR}/*.cpp  CONFIGURE_DEPEND)
target_sources(geos PRIVATE ${_sources})
unset(_sources)
