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

file(GLOB_RECURSE geos_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(geos SHARED ${geos_SOURCES})
add_library(geos-static STATIC ${geos_SOURCES})

# TODO: Enable SOVERSION property
set_target_properties(geos
  PROPERTIES
  DEFINE_SYMBOL GEOS_DLL_EXPORT
  VERSION ${VERSION}
  CLEAN_DIRECT_OUTPUT 1)

set_target_properties(geos-static
  PROPERTIES
  OUTPUT_NAME "geos"
  PREFIX "lib"
  CLEAN_DIRECT_OUTPUT 1)

install(TARGETS geos geos-static
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
