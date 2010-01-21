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
# Check custom global options
#################################################################################

option(GEOS_ENABLE_INLINE
  "Set to ON to enable GEOS compilation with small functions inlining" ON)

option(GEOS_ENABLE_ASSERT
  "Set to ON to build GEOS with assert() macro enabled" OFF) 

#################################################################################
# Setup C/C++ compiler options
#################################################################################

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release" FORCE)
endif()
message(STATUS "Setting GEOS build type - ${CMAKE_BUILD_TYPE}")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions(-D_DEBUG)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)

  # General options
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -ansi")

  # Warnings specification
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long")

  # Turn on Position Independent Code generation for GEOS C shared library
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

elseif(MSVC)
    
  # Set pedantic mode by default
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

  if(MSVC80 OR MSVC90 OR MSVC10)

    # Option is to enable the /MP switch for Visual Studio 2005 or later
    option(GEOS_MSVC_ENABLE_MP
      "Set to ON to build GEOS with the /MP option (Visual Studio 2005 and above)." OFF)
    mark_as_advanced(GEOS_MSVC_ENABLE_MP)
    if(GEOS_MSVC_ENABLE_MP)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
    message(STATUS "Setting Visual Studio 2005+ option /MP to ${ENABLE_MSVC_MP}")
    
    # Disable C4996: 'function': was declared deprecated
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  endif()

endif()

if(GEOS_ENABLE_INLINE)
  add_definitions(-DGEOS_INLINE)
endif()
message(STATUS
  "Setting GEOS compilation with small functions inlining - ${GEOS_ENABLE_INLINE}")

if(GEOS_ENABLE_ASSERT AND CMAKE_BUILD_TYPE MATCHES "Rel")
  string(REPLACE "-DNDEBUG" "-UNDEBUG"
    CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()
message(STATUS
  "Setting GEOS compilation with assert() macro enabled - ${GEOS_ENABLE_ASSERT}")

#################################################################################
# Setup C/C++ library features
#################################################################################

include(TestForSTDNamespace)
if (CMAKE_NO_STD_NAMESPACE)
  message(FATAL_ERROR "Detected compiler does not support std namespace on STL classes")
endif()

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

# TODO: This needs to be improved --mloskot
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

# for including GEOS C++ API headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# for including build-specific GEOS C API headers
include_directories(${CMAKE_BINARY_DIR}/capi)

# for including build-specific version.h, platform.h and geos_c.h
include_directories(${CMAKE_BINARY_DIR}/include)

#################################################################################
# Setup checks and generate config headers and geos-config tool
#################################################################################

# TODO: output to CMAKE_CURRENT_BINARY_DIR instead of CMAKE_SOURCE_DIR

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/geos/platform.h.cmake 
  ${CMAKE_CURRENT_BINARY_DIR}/include/geos/platform.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/geos/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/geos/version.h @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/capi/geos_c.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/capi/geos_c.h @ONLY)

#################################################################################
# Configure subdirectories
#################################################################################

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(capi)
add_subdirectory(tests)
add_subdirectory(tools)

#################################################################################
# Setup installation of common files not specific to any target


#################################################################################
# DEBUG settings - TODO: make a summary
message(STATUS "XXX: CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
message(STATUS "XXX: CMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "XXX: CMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}")
