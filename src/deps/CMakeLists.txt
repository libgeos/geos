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
file(GLOB_RECURSE _sources ${CMAKE_CURRENT_LIST_DIR}/*.c  CONFIGURE_DEPEND)
add_library(ryu OBJECT ${_sources})
target_include_directories(ryu PUBLIC ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(ryu PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  C_STANDARD 99
)
unset(_sources)

target_include_directories(geos
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

