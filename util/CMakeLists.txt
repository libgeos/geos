################################################################################
# Part of CMake configuration for GEOS
#
# Copyright (C) 2020 Martin Davis
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
################################################################################

option(BUILD_GEOSOP "Build geosop command-line interface" ON)
message(STATUS "GEOS: Build geosop ${BUILD_GEOSOP}")

if(BUILD_GEOSOP)

add_executable(orientview
  OrientationIndexViewer.cpp
  )

target_link_libraries(orientview PRIVATE geos geos_c)

add_subdirectory(geosop)

endif()
