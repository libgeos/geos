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
add_executable(perf_interiorpoint_area InteriorPointAreaPerfTest.cpp)
target_link_libraries(perf_interiorpoint_area geos)

add_executable(perf_voronoi VoronoiPerfTest.cpp)
target_link_libraries(perf_voronoi geos)

add_executable(perf_unaryunion_segments UnaryUnionSegmentsPerfTest.cpp)
target_link_libraries(perf_unaryunion_segments geos)
