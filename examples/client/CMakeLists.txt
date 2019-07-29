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

project(GEOS VERSION 1.0.0 LANGUAGES C CXX)
find_package(GEOS 3.8 REQUIRED)

add_executable(geos_client geos_client.cpp)
target_link_libraries(geos_client PRIVATE GEOS::geos)

add_executable(geos_c_client geos_c_client.cpp)
target_link_libraries(geos_c_client PRIVATE GEOS::geos_c)
