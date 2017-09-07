#################################################################################
#
# CMake configuration for GEOS perf/operation/buffer tests
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

add_executable(perf_iterated_buffer IteratedBufferStressTest.cpp)

target_link_libraries(perf_iterated_buffer geos)

add_test(perf_iterated_buffer
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/perf_iterated_buffer)
