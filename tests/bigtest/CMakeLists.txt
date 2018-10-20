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
add_executable(test_bug234 bug234.cpp)
target_link_libraries(test_bug234 PRIVATE geos)
add_test(NAME test_bug234 COMMAND test_bug234)

add_executable(test_sweep_line_speed "")
target_sources(test_sweep_line_speed
  PRIVATE
    TestSweepLineSpeed.cpp
    GeometryTestFactory.cpp)
target_link_libraries(test_sweep_line_speed PRIVATE geos)
