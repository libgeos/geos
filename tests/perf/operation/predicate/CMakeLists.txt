#################################################################################
#
# CMake configuration for GEOS perf/operation/predicate tests
#
# Copyright (C) 2017 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#
#################################################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/io/markup)

add_executable(perf_rectangle_intersects RectangleIntersectsPerfTest.cpp)

target_link_libraries(perf_rectangle_intersects geos)

add_test(perf_rectangle_intersects
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/perf_rectangle_intersects)
