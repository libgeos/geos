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
add_executable(test_perf_memleak_mp_prep memleak_mp_prep.c)
# test_perf_memleak_mp_prep is not dependant against geos target,
# but geos_c only, so need explicit include directories.
target_include_directories(test_perf_memleak_mp_prep
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>)
target_link_libraries(test_perf_memleak_mp_prep PRIVATE geos_c)
add_test(NAME test_perf_memleak_mp_prep COMMAND test_perf_memleak_mp_prep)
