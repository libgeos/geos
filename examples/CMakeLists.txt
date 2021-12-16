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
cmake_minimum_required(VERSION 3.12)

project(geos-examples VERSION 1.0.0 LANGUAGES C CXX)
find_package(GEOS 3.10 REQUIRED)

add_executable(capi_read capi_read.c)
target_link_libraries(capi_read PRIVATE GEOS::geos_c)

add_executable(capi_prepared capi_prepared.c)
target_link_libraries(capi_prepared PRIVATE GEOS::geos_c)

add_executable(capi_strtree capi_strtree.c)
target_link_libraries(capi_strtree PRIVATE GEOS::geos_c)

add_executable(capi_read_ts capi_read_ts.c)
target_link_libraries(capi_read_ts PRIVATE GEOS::geos_c)

add_executable(cpp_strtree cpp_strtree.cpp)
target_link_libraries(cpp_strtree PRIVATE GEOS::geos)

add_executable(cpp_read cpp_read.cpp)
target_link_libraries(cpp_read PRIVATE GEOS::geos)

