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

if(GEOS_BUILD_FROM_GIT)
  add_custom_target(geos_revision_header
    COMMAND ${CMAKE_COMMAND}
    -D DST=${CMAKE_BINARY_DIR}/geos_revision.h
    -D CMAKE_MODULE_PATH="${PROJECT_SOURCE_DIR}/cmake"
    -P ${CMAKE_SOURCE_DIR}/cmake/GenerateRevisionHeader.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
else()
  add_library(geos_revision_header INTERFACE)
endif()

add_dependencies(geos_c geos_revision_header)

file(GLOB_RECURSE _headers ${CMAKE_CURRENT_LIST_DIR}/*.cpp  CONFIGURE_DEPEND)
target_sources(geos_c PRIVATE ${_headers})
unset(_headers)

target_include_directories(geos_c
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/geos>)

set(VERSION ${GEOS_VERSION})
set(VERSION_MAJOR ${GEOS_VERSION_MAJOR})
set(VERSION_MINOR ${GEOS_VERSION_MINOR})
set(VERSION_PATCH ${GEOS_VERSION_PATCH})

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/geos_c.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/geos_c.h
  @ONLY)

unset(VERSION)
unset(VERSION_MAJOR)
unset(VERSION_MINOR)
unset(VERSION_PATCH)
