#################################################################################
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

if(NOT CMAKE_VERSION)
    set(CMAKE_VERSION
      "${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")
endif()

# Location of custom CMake modules with macros used by GEOS
set(CMAKE_MODULE_PATH "${geos_SOURCE_DIR}/cmake/modules")

#################################################################################
# Setup GEOS version
#################################################################################

# GEOS release version
# GEOS C++ library SONAME will use these encoding ABI break at every release
set(VERSION_MAJOR 3)
set(VERSION_MINOR 5)
set(VERSION_PATCH 0)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# JTS_PORT is the version of JTS this release is bound to
set(JTS_PORT 1.13.0)
message(STATUS "Setting GEOS version ${VERSION} as port of JTS ${JTS_PORT}")

# GEOS C API version
set(CAPI_INTERFACE_CURRENT 10)
set(CAPI_INTERFACE_REVISION 0)
set(CAPI_INTERFACE_AGE 9)

math(EXPR CAPI_VERSION_MAJOR "${CAPI_INTERFACE_CURRENT} - ${CAPI_INTERFACE_AGE}")
set(CAPI_VERSION_MINOR ${CAPI_INTERFACE_AGE})
set(CAPI_VERSION_PATCH ${CAPI_INTERFACE_REVISION})
set(CAPI_VERSION "${CAPI_VERSION_MAJOR}.${CAPI_VERSION_MINOR}.${CAPI_VERSION_PATCH}")
message(STATUS "Setting GEOS C API version ${CAPI_VERSION}")
if (NOT WIN32)
  set(CAPI_SOVERSION ${CAPI_VERSION_MAJOR})
  message(STATUS "Setting GEOS C API soversion ${CAPI_SOVERSION}")
endif()

#################################################################################
# Check custom global options
#################################################################################

option(GEOS_ENABLE_TESTS
  "Set to OFF|ON (default) to control build of GEOS tests package" ON)

option(GEOS_ENABLE_INLINE
  "Set to OFF|ON (default) to control GEOS compilation with small functions inlining" ON)

if(NOT MSVC)
  option(GEOS_ENABLE_ASSERT
    "Set to ON|OFF (default) to build GEOS with assert() macro enabled" OFF) 
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  option(GEOS_ENABLE_FLOATSTORE
    "Set to OFF|ON (default) to control IEEE754 conformance and remove extra precision" ON)
endif()

if(APPLE)
  option(GEOS_ENABLE_MACOSX_FRAMEWORK
    "Set to ON|OFF (default) to build GEOS as a Mac OS X framework" OFF)
  option(GEOS_ENABLE_MACOSX_FRAMEWORK_UNIXCOMPAT
    "Set to ON|OFF (default) to add Unix compatibility to the Mac OS X framework" OFF)
endif()

#################################################################################
# Setup C/C++ compiler options
#################################################################################

if(NOT MSVC_IDE)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release" FORCE)
  endif()
  message(STATUS "Setting GEOS build type - ${CMAKE_BUILD_TYPE}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions(-D_DEBUG)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)

  # General options
  set(CMAKE_CXX_FLAGS "-pedantic -ansi ${CMAKE_CXX_FLAGS}")

  # Numerical stability
  if(GEOS_ENABLE_FLOATSTORE)
    # Remove extra precision by forcing conformance to IEEE 754 rather than IEEE 854
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffloat-store")
  endif()
  message(STATUS
    "Forcing IEEE 754 using flag -ffloat-store - ${GEOS_ENABLE_FLOATSTORE}")

  # Warnings specification
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long")

  # Turn on Position Independent Code generation for GEOS C shared library
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

  # Enable glibc ISO C99 features (macros isfinite, isnan)
  set(CMAKE_REQUIRED_DEFINITIONS "${CMAKE_REQUIRED_DEFINITIONS} -D_ISOC99_SOURCE=1")

elseif(MSVC)
    
  # Set pedantic mode by default
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  string(REGEX REPLACE "/W[0-9]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

  if(MSVC80 OR MSVC90 OR MSVC10 OR MSVC11 OR MSVC12 OR MSVC13)

    # Option is to enable the /MP switch for Visual Studio 2005 or later
    option(GEOS_MSVC_ENABLE_MP
      "Set to ON to build GEOS with the /MP option (Visual Studio 2005 and above)." ON)
    mark_as_advanced(GEOS_MSVC_ENABLE_MP)
    if(GEOS_MSVC_ENABLE_MP)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
    message(STATUS "Setting Visual Studio 2005+ option /MP to ${GEOS_MSVC_ENABLE_MP}")
    
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
  endif()

endif()

if(GEOS_ENABLE_INLINE)
  add_definitions(-DGEOS_INLINE)
endif()
message(STATUS
  "Setting GEOS compilation with small functions inlining - ${GEOS_ENABLE_INLINE}")

if(NOT MSVC)
  if(GEOS_ENABLE_ASSERT)
    string(REGEX REPLACE "[-/]D.*NDEBUG" "-U NDEBUG"
      CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  endif()
  message(STATUS
    "Setting GEOS compilation with assert() macro enabled - ${GEOS_ENABLE_ASSERT}")
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
include(CheckPrototypeExists)
include(CheckSymbolExists)

check_prototype_exists(isnan cmath HAVE_STD_ISNAN)
if(NOT HAVE_STD_ISNAN)
  if(MSVC)
    check_prototype_exists(_isnan float.h HAVE_ISNAN)
  elseif(APPLE)
    check_prototype_exists(__isnand math.h HAVE_ISNAND_XCODE)
    if(NOT HAVE_ISNAND_XCODE)
      check_prototype_exists(__inline_isnand math.h HAVE_INLINE_ISNAND_XCODE)
    endif()
  else()
    check_symbol_exists(isnan math.h HAVE_ISNAN)
  endif()
endif()

check_prototype_exists(isfinite cmath HAVE_STD_ISFINITE)

if(NOT HAVE_STD_ISFINITE)
  if(MSVC)
    check_prototype_exists(_finite float.h HAVE_FINITE)
  else()
    #CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    check_symbol_exists(isfinite math.h HAVE_ISFINITE)
  endif()
endif()

################################################################################
# Setup build directories
#################################################################################

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf
# directories. This simplifies manual testing and the use of the build
# tree rather than installed Boost libraries.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

################################################################################
# Setup include directories
#################################################################################

# for including GEOS C++ API headers
include_directories(${geos_SOURCE_DIR}/include)

# for including build-specific GEOS C API headers
include_directories(${geos_BINARY_DIR}/capi)

# for including build-specific version.h, platform.h and geos_c.h
include_directories(${geos_BINARY_DIR}/include)

#################################################################################
# Setup checks and generate config headers
#################################################################################

#################################################################################
#  MACRO: GET_SVN_REVISION
#  
#  DESCRIPTION:
#      MACRO FOR GETTING THE SVN revision for this build
#################################################################################
MACRO (GET_SVN_REVISION)
   FIND_PACKAGE(Subversion)
   IF(SUBVERSION_FOUND)
      Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
      # MESSAGE("Current revision is ${Project_WC_REVISION}")
      # Subversion_WC_LOG(${PROJECT_SOURCE_DIR} Project)
      # MESSAGE("Last changed log is ${Project_LAST_CHANGED_LOG}")
   ENDIF()
ENDMACRO(GET_SVN_REVISION)

# Determine SVN/Git revision
if(EXISTS "${CMAKE_SOURCE_DIR}/.svn")
  GET_SVN_REVISION()
endif()
if ( NOT ${Project_WC_REVISION} EQUAL 0 )
   set( GEOS_SVN_REVISION ${Project_WC_REVISION} )
   configure_file ( 
      "${PROJECT_SOURCE_DIR}/tools/geos_svn_revision_cmake.h.in"
      "${PROJECT_SOURCE_DIR}/geos_svn_revision.h" )
      # "${geos_BINARY_DIR}/include/geos_svn_revision.h" )
else()
   message(STATUS "Generating revision header ${CMAKE_SOURCE_DIR}/geos_svn_revision.h")
   find_program(SH sh)
   if(SH)
      execute_process(COMMAND ${SH} -c 
      "cd ${CMAKE_SOURCE_DIR} && ${CMAKE_SOURCE_DIR}/tools/svn_repo_revision.sh")

      file(RENAME "${CMAKE_SOURCE_DIR}/geos_svn_revision.h"
      "${CMAKE_BINARY_DIR}/geos_svn_revision.h")
   else()
      message("*** sh-compatible command not found, cannot create geos_svn_revision.h")
      message("*** Check SVN revision and create revision header manually:")
      message("*** echo '#define GEOS_SVN_REVISION XYZ' > ${CMAKE_SOURCE_DIR}/geos_svn_revision.h")
   endif()
endif()
# End: Determine SVN/Git revision

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/geos/platform.h)
  message(STATUS "Disabling existing ${CMAKE_CURRENT_SOURCE_DIR}/include/geos/platform.h")

  if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/include/geos/platform.h)
    set(PH_RESULT "removed")
  else()
    file(RENAME
      ${CMAKE_CURRENT_SOURCE_DIR}/include/geos/platform.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/geos/platform.h.disabled)
      set(PH_RESULT "renamed")
  endif()

  message(STATUS "Disabling existing ${CMAKE_CURRENT_SOURCE_DIR}/include/geos/platform.h - ${PH_RESULT}")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/geos/platform.h.cmake 
  ${CMAKE_CURRENT_BINARY_DIR}/include/geos/platform.h)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/geos/version.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/include/geos/version.h @ONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/capi/geos_c.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/capi/geos_c.h @ONLY)

#################################################################################
# Configure tests
#################################################################################

if(GEOS_ENABLE_TESTS)

 # Include CTest support
 include(CTest)
 enable_testing()

  # Define "make check" as alias for "make test"
  add_custom_target(check COMMAND ctest)

endif()

#################################################################################
# Configure subdirectories
#################################################################################
include(GenerateSourceGroups)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(capi)
add_subdirectory(tests)
add_subdirectory(tools)

#################################################################################
# Install/Uninstall
#################################################################################

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake") 

#################################################################################
# DEBUG settings - TODO: make a summary

message(STATUS "CMake ${CMAKE_VERSION} successfully configured ${PROJECT_NAME} using ${CMAKE_GENERATOR} generator")

#message(STATUS "XXX: CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
#message(STATUS "XXX: CMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}")
#message(STATUS "XXX: CMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}")
