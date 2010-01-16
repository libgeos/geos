#################################################################################
# $Id$
#
# Main GEOS build configuration file for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################
project(geos)
cmake_minimum_required(VERSION 2.6)

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#################################################################################
# Configure GEOS version
#################################################################################

# GEOS release version
# GEOS C++ library SONAME will use these encoding ABI break at every release
set(VERSION_MAJOR 3)
set(VERSION_MINOR 3)
set(VERSION_PATCH 0)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# JTS_PORT is the version of JTS this release is bound to
set(JTS_PORT 1.10.0)
message(STATUS "Setting GEOS version ${VERSION} as port of JTS ${JTS_PORT}")

# GEOS C API version
set(CAPI_INTERFACE_CURRENT 7)
set(CAPI_INTERFACE_REVISION 1)
set(CAPI_INTERFACE_AGE 6)

math(EXPR CAPI_VERSION_MAJOR "${CAPI_INTERFACE_CURRENT} - ${CAPI_INTERFACE_AGE}")
set(CAPI_VERSION_MINOR ${CAPI_INTERFACE_AGE})
set(CAPI_VERSION_PATCH ${CAPI_INTERFACE_REVISION})
set(CAPI_VERSION "${CAPI_VERSION_MAJOR}.${CAPI_VERSION_MINOR}.${CAPI_VERSION_PATCH}")
message(STATUS "Setting GEOS C API version ${CAPI_VERSION}")

# TODO: consider setting in ${CMAKE_CURRENT_BINARY_DIR}/include/geos/version.h )
configure_file(${CMAKE_SOURCE_DIR}/include/geos/version.h.cmake 
  ${CMAKE_SOURCE_DIR}/include/geos/version.h )

#################################################################################
# Configure C/C++ compiler options
#################################################################################

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wno-long-long -ansi")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

#################################################################################
# Configure subdirectories
#################################################################################

add_subdirectory(src)
add_subdirectory(capi)
