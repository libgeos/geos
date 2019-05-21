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
add_executable(test_perf_rectangle_intersects RectangleIntersectsPerfTest.cpp)
target_link_libraries(test_perf_rectangle_intersects PRIVATE geos)
add_test(NAME test_perf_rectangle_intersects COMMAND test_perf_rectangle_intersects)
