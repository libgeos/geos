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
cmake_minimum_required(VERSION 3.5)

# TODO: Follow CMake detection of git and version tagging
#       https://gitlab.kitware.com/cmake/cmake/blob/master/Source/CMakeVersionSource.cmake
if(EXISTS ${CMAKE_SOURCE_DIR}/.git/HEAD)
  set(GEOS_BUILD_FROM_GIT ON)
endif()

# TODO: Remove when autotools is dropped and configured headers
#       accept native CMake variables (eg. GEOS_MAJOR_VERSION)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(geos-configure-file)

################################################################################
# Options
include(CMakeDependentOption)
## CMake global variables
option(BUILD_SHARED_LIBS "Build GEOS with shared libraries" OFF)
## GEOS custom variables
cmake_dependent_option(GEOS_BUILD_DEVELOPER
  "Build with compilation flags useful for development" ON
  "GEOS_BUILD_FROM_GIT" OFF)
mark_as_advanced(GEOS_BUILD_DEVELOPER)
#TODO: option(GEOS_BUILD_OSX_FRAMEWORK "Build shared or static library as Framework Bundle for OSX" OFF)
#mark_as_advanced(GEOS_BUILD_OSX_FRAMEWORK)

################################################################################
# Project settings
if(NOT CMAKE_VERSION VERSION_LESS 3.12)
  set(_project_extras HOMEPAGE_URL;"http://geos.osgeo.org")
endif()

project(GEOS VERSION 3.8.0 LANGUAGES C CXX
  DESCRIPTION "GEOS - C++ port of the Java Topology Suite (JTS)"
  ${_project_extras})

unset(_project_url)

# The version of JTS this release is bound to
set(JTS_PORT "1.13.0")
# Version info for the CAPI
set(CAPI_INTERFACE_CURRENT 12)
set(CAPI_INTERFACE_REVISION 0)
set(CAPI_INTERFACE_AGE 11)

################################################################################
# C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

################################################################################
# Target geos_cxx_flags: common C++ compilation flags
add_library(geos_cxx_flags INTERFACE)
target_compile_features(geos_cxx_flags INTERFACE cxx_std_11)

# geos_cxx_flags inherits properties from geos_developer_cxx_flags
# when building as part of the GEOS repository, in developer mode.
# The flags are intended only for GEOS itself and are not part of
# usage requirements needed by GEOS consumers.
if(GEOS_BUILD_DEVELOPER)
  target_link_libraries(geos_cxx_flags
    INTERFACE
      $<BUILD_INTERFACE:geos_developer_cxx_flags>)
endif()

# geos_developer_cxx_flags is used ONLY BY targets that are built
# as part of the GEOS repository, in developer mode.
add_library(geos_developer_cxx_flags INTERFACE)
target_compile_definitions(geos_developer_cxx_flags
  INTERFACE
    "USE_UNSTABLE_GEOS_CPP_API")

# TODO: Replace the old-school ugly if-conditions with generator expressions
#   For example:
#     $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
#     $<$<CXX_COMPILER_ID:GNU>:-Wall;-Wextra;-Wpedantic>
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(geos_developer_cxx_flags INTERFACE "-W4")
  target_compile_definitions(geos_developer_cxx_flags
    INTERFACE
      "_SCL_SECURE_NO_WARNINGS"
      "_CRT_SECURE_NO_WARNINGS"
      "NOMINMAX")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # Warnings specification
  target_compile_options(geos_developer_cxx_flags
    INTERFACE
      -pedantic
      -Wall -Wextra -Wno-long-long -Wcast-align -Wconversion -Wchar-subscripts
      -Wdouble-promotion -Wpointer-arith -Wformat -Wformat-security -Wshadow
      -Wuninitialized -Wunused-parameter
      -fno-common
      $<$<CXX_COMPILER_ID:GNU>:-fno-implicit-inline-templates>)
endif()

################################################################################
# Target geos: C++ API library
add_library(geos "")
target_link_libraries(geos PUBLIC geos_cxx_flags)
target_compile_definitions(geos PRIVATE USE_UNSTABLE_GEOS_CPP_API)
add_subdirectory(include)
add_subdirectory(src)

if(BUILD_SHARED_LIBS)
  set_target_properties(geos PROPERTIES VERSION ${GEOS_VERSION})
  if(NOT WIN32)
    set_target_properties(geos PROPERTIES SOVERSION ${GEOS_VERSION_MAJOR})
  endif()
endif()

################################################################################
# Target geos_c: C API library
add_library(geos_c "")
target_link_libraries(geos_c PRIVATE geos)
add_subdirectory(capi)

if(BUILD_SHARED_LIBS)
  set_target_properties(geos_c PROPERTIES VERSION ${GEOS_VERSION})
  if(NOT WIN32)
    set_target_properties(geos_c PROPERTIES SOVERSION ${GEOS_VERSION_MAJOR})
  endif()
endif()

################################################################################
# Tests
include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

################################################################################
# Install and export targets - support 'make install' or equivalent
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config-version.cmake"
  VERSION ${GEOS_VERSION}
  COMPATIBILITY AnyNewerVersion)

configure_file(cmake/geos-config.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config.cmake"
  COPYONLY)

install(TARGETS geos geos_c geos_cxx_flags
  EXPORT geos-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include)

install(EXPORT geos-targets
  FILE geos-targets.cmake
  NAMESPACE GEOS::
  DESTINATION lib/cmake/GEOS)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config-version.cmake"
  DESTINATION lib/cmake/GEOS)
install(DIRECTORY
  "${CMAKE_CURRENT_LIST_DIR}/include/geos"
  "${CMAKE_CURRENT_BINARY_DIR}/include/geos"
  DESTINATION include
  FILES_MATCHING PATTERN "*.h")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/capi/geos_c.h"
  DESTINATION include)

################################################################################
# Export targets - support target consumption from the build directory
#TODO: export(EXPORT geos-targets NAMESPACE GEOS::)

################################################################################
# Summary
message(STATUS "${PROJECT_NAME} configuration summary:")
message(STATUS "  Version:                     ${GEOS_VERSION}")
message(STATUS "  JTS port:                    ${JTS_PORT}")
message(STATUS "  BUILD_SHARED_LIBS:           ${BUILD_SHARED_LIBS}")
message(STATUS "  BUILD_TESTING:               ${BUILD_TESTING}")
message(STATUS "  GEOS_BUILD_DEVELOPER:        ${GEOS_BUILD_DEVELOPER}")
