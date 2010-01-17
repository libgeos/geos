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
# Setup GEOS version
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

#################################################################################
# Setup C/C++ compiler options
#################################################################################

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wno-long-long -ansi")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

#################################################################################
# Setup C/C++ library features
#################################################################################

# check header files
include(CheckIncludeFiles)

check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(ieeefp.h HAVE_IEEEFP_H)

# check types and sizes
include(CheckTypeSize)

if(MSVC)
  check_type_size("__int64" HAVE_INT64_T_64)
else()
  if(HAVE_STDINT_H OR HAVE_INTTYPES_H)
    check_type_size("int64_t" HAVE_INT64_T_64)
  else()
    check_type_size("long long int" HAVE_LONG_LONG_INT_64)
  endif()
endif()

# check functions and macros
include(CheckSymbolExists)
include(CheckFunctionExists)

check_symbol_exists(std::isnan cmath HAVE_STD_ISNAN)
if(NOT HAVE_STD_ISNAN)
  if(MSVC)
    check_symbol_exists(_isnan float.h HAVE_ISNAN)
  elseif(APPLE)
    check_symbol_exists(__isnand math.h HAVE_ISNAND_XCODE)
    if(NOT HAVE_ISNAND_XCODE)
      check_symbol_exists(__inline_isnand math.h HAVE_INLINE_ISNAND_XCODE)
    endif()
  else()
    check_symbol_exists(isnan math.h HAVE_ISNAN)
  endif()
endif()

check_symbol_exists(std::isfinite cmath HAVE_STD_ISFINITE)
if(NOT HAVE_STD_ISFINITE)
  if(MSVC)
    check_symbol_exists(_finite float.h HAVE_FINITE)
  else()
    check_symbol_exists(isfinite math.h HAVE_ISFINITE)
  endif()
endif()

################################################################################
# Setup include directories
#################################################################################

# for including build-specific version.h and platform.h
include_directories(${CMAKE_BINARY_DIR}/include)

#################################################################################
# Setup checks and generate config headers
#################################################################################

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/geos/version.h.cmake 
  ${CMAKE_CURRENT_BINARY_DIR}/include/geos/version.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/geos/platform.h.cmake 
  ${CMAKE_CURRENT_BINARY_DIR}/include/geos/platform.h)

#################################################################################
# Configure subdirectories
#################################################################################

add_subdirectory(src)
add_subdirectory(capi)
