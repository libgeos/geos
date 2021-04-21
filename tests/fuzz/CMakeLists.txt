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
if(DEFINED ENV{LIB_FUZZING_ENGINE})
  add_executable(fuzz_geo2 fuzz_geo2.c)
  target_include_directories(fuzz_geo2 PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
  target_link_libraries(fuzz_geo2 geos_c $ENV{LIB_FUZZING_ENGINE})
endif()
