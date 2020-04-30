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
if(DISABLE_GEOS_INLINE)
  file(GLOB_RECURSE _headers ${CMAKE_CURRENT_LIST_DIR}/*.h  CONFIGURE_DEPEND)
else()
  file(GLOB_RECURSE _headers ${CMAKE_CURRENT_LIST_DIR}/*.h  ${CMAKE_CURRENT_LIST_DIR}/*.inl CONFIGURE_DEPEND)
endif()
target_sources(geos PRIVATE ${_headers})
unset(_headers)

target_include_directories(geos
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/geos>)

set(VERSION ${GEOS_VERSION})
set(VERSION_MAJOR ${GEOS_VERSION_MAJOR})
set(VERSION_MINOR ${GEOS_VERSION_MINOR})
set(VERSION_PATCH ${GEOS_VERSION_PATCH})

configure_file(${CMAKE_CURRENT_LIST_DIR}/geos/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/geos/version.h
  @ONLY)

unset(VERSION)
unset(VERSION_MAJOR)
unset(VERSION_MINOR)
unset(VERSION_PATCH)
