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
find_package(Git REQUIRED)

file(WRITE ${CMAKE_BINARY_DIR}/geos_revision.h.in
"\#define GEOS_REVISION \"@GEOS_REVISION@\"\n")

file(WRITE ${CMAKE_BINARY_DIR}/geos_revision.cmake
"
execute_process(COMMAND \${GIT} describe --tags --always
  WORKING_DIRECTORY \${CWD}
  OUTPUT_VARIABLE GEOS_REVISION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
configure_file(\${SRC} \${DST} @ONLY)
")

add_custom_target(geos_revision
  COMMAND ${CMAKE_COMMAND}
    -D CWD=${CMAKE_CURRENT_SOURCE_DIR}
    -D GIT=${GIT_EXECUTABLE}
    -D SRC=${PROJECT_BINARY_DIR}/geos_revision.h.in
    -D DST=${PROJECT_BINARY_DIR}/geos_revision.h
    -P ${PROJECT_BINARY_DIR}/geos_revision.cmake)

add_dependencies(geos_c geos_revision)

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

math(EXPR CAPI_VERSION_MAJOR "${CAPI_INTERFACE_CURRENT} - ${CAPI_INTERFACE_AGE}")
set(CAPI_VERSION_MINOR ${CAPI_INTERFACE_AGE})
set(CAPI_VERSION_PATCH ${CAPI_INTERFACE_REVISION})
set(CAPI_VERSION "${CAPI_VERSION_MAJOR}.${CAPI_VERSION_MINOR}.${CAPI_VERSION_PATCH}")

geos_configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/geos_c.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/geos_c.h
  @ONLY)

unset(VERSION)
unset(VERSION_MAJOR)
unset(VERSION_MINOR)
unset(VERSION_PATCH)
