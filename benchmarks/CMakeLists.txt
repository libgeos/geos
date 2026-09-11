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

find_package(benchmark QUIET)

add_executable(perf_class_sizes ClassSizes.cpp)
target_include_directories(perf_class_sizes PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
target_compile_features(perf_class_sizes PRIVATE cxx_std_17)
target_compile_options(perf_class_sizes PRIVATE ${geos_cxx_private_flags} ${geos_cxx_interface_flags})
target_compile_definitions(perf_class_sizes PRIVATE ${geos_cxx_private_definitions} ${geos_cxx_interface_definitions})

add_subdirectory(capi)

add_subdirectory(algorithm)
add_subdirectory(geom)
add_subdirectory(index)
add_subdirectory(operation)
