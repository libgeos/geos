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

set_target_properties(geos-static PROPERTIES OUTPUT_NAME "geos")
set_target_properties(geos PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(geos-static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# TODO: Enable SOVERSION property
set_target_properties(geos PROPERTIES VERSION ${VERSION})

install(TARGETS geos
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
