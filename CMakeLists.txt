##############################################################################
# Part of CMake configuration for GEOS
#
# Copyright (C) 2018-2019 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
##############################################################################

# Require CMake 3.13+ with support for meta-features that request compiler
# modes for specific C/C++ language standard levels, and object libraries.
cmake_minimum_required(VERSION 3.13)

#-----------------------------------------------------------------------------
# Version
#-----------------------------------------------------------------------------
file(READ Version.txt _version_txt)

string(REGEX MATCH "GEOS_VERSION_MAJOR=([0-9]+)" _ ${_version_txt})
set(_version_major ${CMAKE_MATCH_1})
string(REGEX MATCH "GEOS_VERSION_MINOR=([0-9]+)" _ ${_version_txt})
set(_version_minor ${CMAKE_MATCH_1})
string(REGEX MATCH "GEOS_VERSION_PATCH=([0-9]+)" _ ${_version_txt})
set(_version_patch ${CMAKE_MATCH_1})
# OPTIONS: "", "dev", "rc1" etc.
string(REGEX MATCH "GEOS_PATCH_WORD=([a-zA-Z0-9]+)" _ ${_version_txt})
set(_version_patch_word ${CMAKE_MATCH_1})

# Version of JTS this release is bound to
string(REGEX MATCH "JTS_PORT=([0-9a-zA-Z\.]+)" _ ${_version_txt})
set(JTS_PORT ${CMAKE_MATCH_1})

# Version of public C API
string(REGEX MATCH "CAPI_INTERFACE_CURRENT=([0-9]+)" _ ${_version_txt})
set(_version_capi_current ${CMAKE_MATCH_1})
string(REGEX MATCH "CAPI_INTERFACE_REVISION=([0-9]+)" _ ${_version_txt})
set(_version_capi_revision ${CMAKE_MATCH_1})
string(REGEX MATCH "CAPI_INTERFACE_AGE=([0-9]+)" _ ${_version_txt})
set(_version_capi_age ${CMAKE_MATCH_1})

unset(_version_txt)

math(EXPR _version_capi_major "${_version_capi_current} - ${_version_capi_age}")
set(CAPI_VERSION_MAJOR ${_version_capi_major})
set(CAPI_VERSION_MINOR ${_version_capi_age})
set(CAPI_VERSION_PATCH ${_version_capi_revision})
set(CAPI_VERSION "${_version_capi_major}.${_version_capi_age}.${_version_capi_revision}")

unset(_version_capi_current)
unset(_version_capi_major)
unset(_version_capi_age)
unset(_version_capi_revision)

#-----------------------------------------------------------------------------
# Project
#-----------------------------------------------------------------------------
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.9)
  list(APPEND _project_info DESCRIPTION "GEOS - C++ port of the Java Topology Suite (JTS)")
endif()
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
  list(APPEND _project_info HOMEPAGE_URL "https://libgeos.org/")
endif()

project(GEOS VERSION "${_version_major}.${_version_minor}.${_version_patch}"
  LANGUAGES C CXX
  ${_project_info})

if(NOT "${_version_patch_word}" STREQUAL "")
  # Re-write VERSION variables after project()
  set(GEOS_VERSION "${GEOS_VERSION}${_version_patch_word}")
  set(GEOS_VERSION_PATCH "${_version_patch}${_version_patch_word}")
endif()
set(GEOS_VERSION_NOPATCH "${_version_major}.${_version_minor}.${_version_patch}")

unset(_version_major)
unset(_version_minor)
unset(_version_patch)
unset(_version_patch_word)

message(STATUS "GEOS: Version ${GEOS_VERSION}")
message(STATUS "GEOS: C API Version ${CAPI_VERSION}")
message(STATUS "GEOS: JTS port ${JTS_PORT}")

if(CMAKE_VERSION VERSION_LESS 3.21)
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(PROJECT_IS_TOP_LEVEL ON)
  else()
    set(PROJECT_IS_TOP_LEVEL OFF)
  endif()
endif()

#-----------------------------------------------------------------------------
# Setup
#-----------------------------------------------------------------------------

# Default to release build so packagers don't release debug builds
set(DEFAULT_BUILD_TYPE Release)

# Require CMake 3.13+ with VS generator for complete support of VS versions
# and support by AppVeyor.
if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# TODO: Follow CMake detection of git and version tagging
#       https://gitlab.kitware.com/cmake/cmake/blob/master/Source/CMakeVersionSource.cmake
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  set(GEOS_BUILD_FROM_GIT ON)
endif()

# Make sure we know our build type
if(NOT CMAKE_BUILD_TYPE)
  get_property(_is_multi_config_generator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if (NOT _is_multi_config_generator)
    set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE})
    message(STATUS "GEOS: Using default build type: ${CMAKE_BUILD_TYPE}")
  endif()
  unset(_is_multi_config_generator)
else()
  message(STATUS "GEOS: Build type: ${CMAKE_BUILD_TYPE}")
endif()

#-----------------------------------------------------------------------------
# Options
#-----------------------------------------------------------------------------
include(Ccache)
include(CMakeDependentOption)

## CMake global variables
option(BUILD_SHARED_LIBS "Build GEOS with shared libraries" ON)
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard version to use (default is 14)")

## GEOS custom variables
option(BUILD_BENCHMARKS "Build GEOS benchmarks" OFF)
cmake_dependent_option(GEOS_BUILD_DEVELOPER
  "Build with compilation flags useful for development" ON
  "GEOS_BUILD_FROM_GIT;PROJECT_IS_TOP_LEVEL" OFF)
mark_as_advanced(GEOS_BUILD_DEVELOPER)

if (POLICY CMP0092)
  # dont set /W3 warning flags by default, we already
  # set /W4 anyway
  cmake_policy(SET CMP0092 NEW)
endif()

#-----------------------------------------------------------------------------
# Setup build directories
#-----------------------------------------------------------------------------
# Place executables and shared libraries in the same location for
# convenience of direct execution from common spot and for
# convenience in environments without RPATH support.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
message(STATUS "GEOS: Run-time output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "GEOS: Archives output: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

#-----------------------------------------------------------------------------
# Add ASAN build option
#-----------------------------------------------------------------------------
#
# To use ASAN with MacOSX you will need to install clang++ from MacPorts
# or HomeBrew, the default XCode one won't work. Then get cmake to use your
# new compiler, with something like.
#
#  cmake \
#   -D CMAKE_C_COMPILER=/opt/local/bin/clang-mp-15 \
#   -D CMAKE_CXX_COMPILER=/opt/local/bin/clang-mp-15++ \
#   -D CMAKE_BUILD_TYPE=ASAN
#
# To suppress reporting on leaks in MacOS itself (yes, really), 
# add a suppression file to your build directory. 
#
#  echo leak:realizeClassWithoutSwift >> lsan.supp
#
# Now run the program with env vars set to turn on leak checking and suppression.
# 
#  MallocNanoZone=0 \
#  ASAN_OPTIONS=detect_leaks=1 \
#  LSAN_OPTIONS=suppressions=lsan.supp \
#  ./bin/test_geos_unit
#

set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer -fno-common")
set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer -fno-common")
set(CMAKE_EXE_LINKER_FLAGS_ASAN "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_SHARED_LINKER_FLAGS_ASAN "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address")

get_property(_cmake_build_type_is_cache CACHE CMAKE_BUILD_TYPE PROPERTY TYPE)
if (_cmake_build_type_is_cache)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "ASAN")
endif()
unset(_cmake_build_type_is_cache)

#-----------------------------------------------------------------------------
# Install directories
#-----------------------------------------------------------------------------

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

#-----------------------------------------------------------------------------
# C++ language version and compilation flags
#-----------------------------------------------------------------------------
message(STATUS "GEOS: Require C++${CMAKE_CXX_STANDARD}")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#-----------------------------------------------------------------------------
# Target geos_cxx_flags: common compilation flags
#-----------------------------------------------------------------------------
add_library(geos_cxx_flags INTERFACE)
target_compile_features(geos_cxx_flags INTERFACE cxx_std_11)

#-----------------------------------------------------------------------------
# Add flags to prevent 'fused multiply-add' operations on targets (ARM64)
# that allow it, as it breaks calculations in DD.cpp.
# TODO: Replace DD calculations with 'long float' where target supports
# true long float, and remove other cases where FMA causes regression
# failures.
#   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=98207
#-----------------------------------------------------------------------------

target_compile_options(geos_cxx_flags INTERFACE
	"$<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-ffp-contract=off>"
	"$<$<CXX_COMPILER_ID:GNU>:-ffp-contract=off>"
	"$<$<BOOL:${MSVC}>:/fp:precise>"
	)

# Use -ffloat-store for 32-bit builds (needed to make some tests pass)
target_compile_options(geos_cxx_flags INTERFACE
  $<$<AND:$<CXX_COMPILER_ID:GNU>,$<EQUAL:4,${CMAKE_SIZEOF_VOID_P}>>:-ffloat-store>
)

# Make sure NDEBUG is defined so that assert() is disabled for
# any non-debug build. Use a generator expression so that this
# works with multi-configuration generators.
target_compile_definitions(geos_cxx_flags INTERFACE $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)

#-----------------------------------------------------------------------------
# Target geos_developer_cxx_flags: developer mode compilation flags
#-----------------------------------------------------------------------------
# Do NOT install this target for end-users!
add_library(geos_developer_cxx_flags INTERFACE)

if(GEOS_BUILD_DEVELOPER)
  message(STATUS "GEOS: Developer mode ENABLED")
endif()

# geos_cxx_flags inherits properties from geos_developer_cxx_flags when
# building as part of the GEOS repository or on explicit request for
# developer compilation mode, as GEOS contributor.
# The flags are intended only for GEOS itself and are not part of
# usage requirements needed by GEOS consumers.
if(GEOS_BUILD_DEVELOPER)
  target_link_libraries(geos_cxx_flags
    INTERFACE
      $<BUILD_INTERFACE:geos_developer_cxx_flags>)
endif()

target_compile_definitions(geos_cxx_flags
  INTERFACE
    USE_UNSTABLE_GEOS_CPP_API)

target_compile_definitions(geos_developer_cxx_flags
  INTERFACE
    $<$<BOOL:${MSVC}>:_CRT_NONSTDC_NO_DEPRECATE>
    $<$<BOOL:${MSVC}>:_SCL_SECURE_NO_DEPRECATE>
    $<$<BOOL:${MSVC}>:_CRT_SECURE_NO_WARNINGS>
    $<$<BOOL:${WIN32}>:NOMINMAX>)

target_compile_options(geos_developer_cxx_flags
  INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:-W4>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Werror -pedantic -Wall -Wextra -Wno-long-long -Wcast-align -Wconversion -Wsign-conversion -Wchar-subscripts -Wdouble-promotion -Wpointer-arith -Wformat -Wformat-security -Wshadow -Wuninitialized -Wunused-parameter -fno-common>
    $<$<CXX_COMPILER_ID:GNU>:-fno-implicit-inline-templates -Wno-psabi>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-unknown-warning-option>
    )

#-----------------------------------------------------------------------------
# Define a coverage build
#-----------------------------------------------------------------------------
set(CMAKE_CXX_FLAGS_COVERAGE "-fprofile-arcs -ftest-coverage")

#-----------------------------------------------------------------------------
# Extra libraries
#-----------------------------------------------------------------------------
include(CheckLibraryExists)
check_library_exists(m pow "" HAVE_LIBM)

#-----------------------------------------------------------------------------
# Target geos: C++ API library
#-----------------------------------------------------------------------------
add_library(geos "")
add_library(GEOS::geos ALIAS geos)
target_link_libraries(geos PUBLIC geos_cxx_flags PRIVATE $<BUILD_INTERFACE:ryu>)
# ryu is an object library, nothing is actually being linked here. The BUILD_INTERFACE
# switch was necessary to build on AppVeyor (CMake 3.16.2) but not locally (CMake 3.16.3)
add_subdirectory(include)
add_subdirectory(src)

# Some packagers would like the DLL files generated by MinGW to
# include version numbers in the file name. Others would rather
# they not (which is the cmake default).
option(VERSION_MINGW_SHARED_LIBS "Add version suffix to MinGW shared libraries" OFF)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(geos
    PRIVATE $<$<BOOL:${WIN32}>:GEOS_DLL_EXPORT>)

  set_target_properties(geos PROPERTIES VERSION ${GEOS_VERSION_NOPATCH})
  set_target_properties(geos PROPERTIES SOVERSION ${GEOS_VERSION_NOPATCH})
  if(MINGW AND VERSION_MINGW_SHARED_LIBS)
    set_target_properties(geos PROPERTIES SUFFIX "-${GEOS_VERSION_NOPATCH}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()
endif()

#-----------------------------------------------------------------------------
# Target geos_c: C API library
#-----------------------------------------------------------------------------
add_library(geos_c "")
add_library(GEOS::geos_c ALIAS geos_c)
target_link_libraries(geos_c PRIVATE geos)

if(BUILD_SHARED_LIBS)
  target_compile_definitions(geos_c
    PRIVATE $<$<BOOL:${WIN32}>:GEOS_DLL_EXPORT>)

  set_target_properties(geos_c PROPERTIES VERSION ${CAPI_VERSION})
  if(NOT WIN32 OR MINGW)
    set_target_properties(geos_c PROPERTIES SOVERSION ${CAPI_VERSION_MAJOR})
  endif()
  if(MINGW AND VERSION_MINGW_SHARED_LIBS)
    set_target_properties(geos_c PROPERTIES SUFFIX "-${CAPI_VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}")
  endif()
endif()

add_subdirectory(capi)

#-----------------------------------------------------------------------------
# Tests
#-----------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()
endif()

#-----------------------------------------------------------------------------
# Benchmarks
#-----------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL AND BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()

#-----------------------------------------------------------------------------
# Utils
#-----------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory(util)
endif()

#-----------------------------------------------------------------------------
# Documentation/Examples
#-----------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory(doxygen)
endif()

#-----------------------------------------------------------------------------
# Web Site
#-----------------------------------------------------------------------------
option(BUILD_WEBSITE "Build website" OFF)

if(PROJECT_IS_TOP_LEVEL AND BUILD_WEBSITE)
  add_subdirectory(web)
endif()

#-----------------------------------------------------------------------------
# Install and export targets - support 'make install' or equivalent
#-----------------------------------------------------------------------------

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config-version.cmake"
  VERSION ${GEOS_VERSION}
  COMPATIBILITY AnyNewerVersion)

configure_file(cmake/geos-config.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config.cmake"
  COPYONLY)

install(TARGETS geos geos_cxx_flags
  EXPORT geos-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

install(TARGETS geos_c
  EXPORT geos-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

install(EXPORT geos-targets
  FILE geos-targets.cmake
  NAMESPACE GEOS::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GEOS)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/geos-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GEOS)
install(DIRECTORY
  "${CMAKE_CURRENT_LIST_DIR}/include/geos"
  "${CMAKE_CURRENT_BINARY_DIR}/include/geos"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h")
install(DIRECTORY
  "${CMAKE_CURRENT_LIST_DIR}/include/geos"
  "${CMAKE_CURRENT_BINARY_DIR}/include/geos"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/capi/geos_c.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES "${CMAKE_CURRENT_LIST_DIR}/include/geos.h"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_subdirectory(tools)

#-----------------------------------------------------------------------------
# Uninstall
#-----------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake")
endif()  # PROJECT_IS_TOP_LEVEL

#-----------------------------------------------------------------------------
# "make dist" workalike
#-----------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
  get_property(_is_multi_config_generator GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if(NOT _is_multi_config_generator)
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GEOS Computational Geometry Library")
    set(CPACK_PACKAGE_VENDOR "OSGeo")
    set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
    set(CPACK_SOURCE_GENERATOR "TBZ2")
    set(CPACK_PACKAGE_VERSION_MAJOR ${GEOS_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${GEOS_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${GEOS_VERSION_PATCH})
    set(CPACK_SOURCE_PACKAGE_FILE_NAME "geos-${GEOS_VERSION}")

    set(CPACK_SOURCE_IGNORE_FILES
      "/\\\\.git"
      "/autogen\\\\.sh"
      "/tools/ci"
      "/HOWTO_RELEASE"
      "/autom4te\\\\.cache"
      "\\\\.yml\$"
      "\\\\.deps"
      "/debian/"
      "/php/"
      "/.*build-.*/"
      "cmake_install\\\\.cmake\$"
      "/include/geos/version\\\\.h\$"
      "/tools/geos-config\$"
      "/tools/geos\\\\.pc\$"
      "/bin/"
      "/web/"
      ${CMAKE_CURRENT_BINARY_DIR}
      )

    # message(STATUS "GEOS: CPACK_SOURCE_PACKAGE_FILE_NAME: ${CPACK_SOURCE_PACKAGE_FILE_NAME}")
    # message(STATUS "GEOS: CPACK_SOURCE_IGNORE_FILES: ${CPACK_SOURCE_IGNORE_FILES}")
    # message(STATUS "GEOS: CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
    include(CPack)
    add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

    message(STATUS "GEOS: Configured 'dist' target")
  endif()
endif()  # PROJECT_IS_TOP_LEVEL

#-----------------------------------------------------------------------------
# "make check" workalike
#-----------------------------------------------------------------------------

if(PROJECT_IS_TOP_LEVEL)
  add_custom_target(check COMMAND ${CMAKE_BUILD_TOOL} test)
endif()

#-----------------------------------------------------------------------------
# "make distcheck" workalike
#-----------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
  if(NOT _is_multi_config_generator)
    find_package(MakeDistCheck)
    AddMakeDistCheck()
    message(STATUS "GEOS: Configured 'distcheck' target")
  endif()

  unset(_is_multi_config_generator)
endif()  # PROJECT_IS_TOP_LEVEL
