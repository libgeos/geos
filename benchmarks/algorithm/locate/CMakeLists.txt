#################################################################################
#
# CMake configuration for GEOS benchmarks/operation/predicate tests
#
# Copyright (C) 2017 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#
#################################################################################

if (benchmark_FOUND)
    add_executable(perf_point_locator PointLocatorPerfTest.cpp)
    target_include_directories(perf_point_locator PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/benchmarks>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
    target_link_libraries(perf_point_locator PRIVATE
            benchmark::benchmark geos)
endif()
