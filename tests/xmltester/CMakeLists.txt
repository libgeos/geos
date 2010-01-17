#################################################################################
# $Id$
#
# GEOS XML tests runner build configuration for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tinyxml)

set(simplewkttester_SOURCES SimpleWKTTester.cpp)

set(xmltester_SOURCES
  XMLTester.cpp
  tinyxml/tinyxml.cpp
  tinyxml/tinystr.cpp
  tinyxml/tinyxmlerror.cpp
  tinyxml/tinyxmlparser.cpp
  BufferResultMatcher.cpp
  SingleSidedBufferResultMatcher.cpp)

add_executable(simplewkttester ${simplewkttester_SOURCES})
target_link_libraries(simplewkttester geos)

add_executable(xmltester ${xmltester_SOURCES})
target_link_libraries(xmltester geos)
