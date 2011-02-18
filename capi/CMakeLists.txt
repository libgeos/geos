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

if(WIN32)
    add_definitions("-DGEOS_DLL_EXPORT=1")
endif()

set(geos_c_SOURCES
  geos_c.cpp
  geos_ts_c.cpp)

add_library(geos_c SHARED ${geos_c_SOURCES})

target_link_libraries(geos_c geos)

# TODO: Enable SOVERSION property
set_target_properties(geos_c
  PROPERTIES
  VERSION ${CAPI_VERSION}
  CLEAN_DIRECT_OUTPUT 1)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/geos_c.h
  DESTINATION include)

install(TARGETS geos_c
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
