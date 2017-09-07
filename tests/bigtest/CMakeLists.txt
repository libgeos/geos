#################################################################################
#
# CMake configuration for GEOS big test
#
# Copyright (C) 2011 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#
#################################################################################

set(STATUS_MESSAGE "Enable GEOS large geometry tests build")
set(STATUS_RESULT "OFF")

if(GEOS_ENABLE_TESTS)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/io/markup)

  add_executable(test_bug234 bug234.cpp)

  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
    target_link_libraries(test_bug234 GEOS)
  else()
    target_link_libraries(test_bug234 geos)
  endif()

  add_executable(test_sweep_line_speed
	TestSweepLineSpeed.cpp
	GeometryTestFactory.cpp
	bigtest.h)
  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
    target_link_libraries(test_sweep_line_speed GEOS)
  else()
    target_link_libraries(test_sweep_line_speed geos)
  endif()

  add_test(test_bug234 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_bug234)
  add_test(test_sweep_line_speed ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_sweep_line_speed)

  set(STATUS_RESULT "ON")
endif()

message(STATUS "${STATUS_MESSAGE} - ${STATUS_RESULT}")
