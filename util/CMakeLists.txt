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
  if(BUILD_SHARED_LIBS)
    if(NOT DEFINED CMAKE_INSTALL_RPATH)
      # Use relative rpath
      if(APPLE)
        set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
      else()
        set(CMAKE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
      endif()
    endif()
    message(STATUS "GEOS: install runtime path for util: ${CMAKE_INSTALL_RPATH}")
  endif()

  add_subdirectory(geosop)
endif()
