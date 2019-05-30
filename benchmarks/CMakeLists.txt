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
add_executable(perf_class_sizes ClassSizes.cpp)
target_link_libraries(perf_class_sizes PRIVATE geos)

add_subdirectory(algorithm)
add_subdirectory(operation)
add_subdirectory(capi)
