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
