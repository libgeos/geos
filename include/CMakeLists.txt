#################################################################################
# $Id$
#
# GEOS C++ library build configuration for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################

#add_subdirectory(geos)

#################################################################################
# Install/Uninstall
#################################################################################

install(FILES geos.h
  DESTINATION include/geos)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos/platform.h
  DESTINATION include/geos)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos/version.h
  DESTINATION include/geos)

install(DIRECTORY geos/
  DESTINATION include/geos
  FILES_MATCHING
  PATTERN "*.h"
  PATTERN "*.inl"
  PATTERN ".svn" EXCLUDE)
