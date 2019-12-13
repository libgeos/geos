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
add_executable(perf_memleak_mp_prep memleak_mp_prep.c)
# test_perf_memleak_mp_prep is not dependant against geos target,
# but geos_c only, so need explicit include directories.
target_include_directories(perf_memleak_mp_prep
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)
target_link_libraries(perf_memleak_mp_prep PRIVATE geos_c)

add_executable(perf_geospreparedcontains GEOSPreparedContainsPerfTest.cpp)
target_link_libraries(perf_geospreparedcontains PRIVATE geos geos_c)

add_executable(perf_intersection IntersectionPerfTest.cpp)
target_link_libraries(perf_intersection PRIVATE geos geos_c)

add_executable(perf_unary UnaryOpPerfTest.cpp)
target_link_libraries(perf_unary PRIVATE geos geos_c)
