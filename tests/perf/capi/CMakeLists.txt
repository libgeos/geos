#################################################################################
#
# CMake configuration for GEOS perf/capi tests
#
# Copyright (C) 2017 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#
#################################################################################

add_executable(perf_memleak_mp_prep memleak_mp_prep.c)

target_link_libraries(perf_memleak_mp_prep geos_c)

add_test(perf_memleak_mp_prep ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/perf_memleak_mp_prep)

add_test(perf_intersection ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/perf_intersection)

add_executable(perf_intersection IntersectionPerfTest.cpp)
target_link_libraries(perf_intersection geos_c)

add_executable(perf_geospreparedcontains GEOSPreparedContainsPerfTest.cpp)
target_link_libraries(perf_geospreparedcontains geos_c)
