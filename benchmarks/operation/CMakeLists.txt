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
add_subdirectory(buffer)
add_subdirectory(predicate)

if (benchmark_FOUND)
    add_executable(perf_distance DistancePerfTest.cpp)
    target_include_directories(perf_distance PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/benchmarks>)
    target_link_libraries(perf_distance PRIVATE
            benchmark::benchmark geos geos_cxx_flags)

    add_executable(perf_coverage_union CoverageUnionPerfTest.cpp)
    target_include_directories(perf_coverage_union PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/benchmarks>)
    target_link_libraries(perf_coverage_union PRIVATE
            benchmark::benchmark geos geos_cxx_flags)
endif()
