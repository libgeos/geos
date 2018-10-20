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
file(GLOB_RECURSE _headers ${CMAKE_CURRENT_LIST_DIR}/*.h  CONFIGURE_DEPEND)
target_sources(geos PRIVATE ${_headers})
unset(_headers)

target_include_directories(geos
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/geos>)

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/geos/platform.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/geos/platform.h)

geos_configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/geos/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/geos/version.h
  @ONLY)
