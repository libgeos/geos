#################################################################################
# $Id$
#
# GEOS unit tests build configuration for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################

include_directories(${CMAKE_SOURCE_DIR}/capi)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tut)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE geos_unit_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(geos_unit ${geos_unit_SOURCES})

target_link_libraries(geos_unit geos geos_c)
