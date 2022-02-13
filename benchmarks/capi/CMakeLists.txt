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
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_link_libraries(perf_memleak_mp_prep PRIVATE geos_c)

add_executable(perf_geospreparedcontains GEOSPreparedContainsPerfTest.cpp)
target_link_libraries(perf_geospreparedcontains PRIVATE geos geos_c)

add_executable(perf_intersection IntersectionPerfTest.cpp)
target_link_libraries(perf_intersection PRIVATE geos geos_c)

add_executable(perf_unary UnaryOpPerfTest.cpp)
target_link_libraries(perf_unary PRIVATE geos geos_c)

if(benchmark_FOUND)
    add_executable(perf_capi_coordseq GEOSCoordSeqPerfTest.cpp)
    target_include_directories(perf_capi_coordseq PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
    target_link_libraries(perf_capi_coordseq PRIVATE
            benchmark::benchmark geos_c)
endif()

if(benchmark_FOUND)
    add_executable(perf_capi_transformxy GEOSGeom_transformXYPerfTest.cpp)
    target_include_directories(perf_capi_transformxy PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
    target_link_libraries(perf_capi_transformxy PRIVATE benchmark::benchmark geos_c)
endif()
