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

add_subdirectory(chain)

IF(benchmark_FOUND)
    add_executable(perf_spatial_index SpatialIndexPerfTest.cpp)
    target_include_directories(perf_spatial_index PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
    target_link_libraries(perf_spatial_index PRIVATE
            benchmark::benchmark geos)
endif()
