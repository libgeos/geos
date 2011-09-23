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

  add_executable(bug234 bug234.cpp)

  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK) 
    target_link_libraries(bug234 GEOS) 
  else() 
    target_link_libraries(bug234 geos) 
  endif() 

  add_executable(TestSweepLineSpeed
	TestSweepLineSpeed.cpp
	GeometryTestFactory.cpp
	bigtest.h)
  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
    target_link_libraries(TestSweepLineSpeed GEOS)
  else()
    target_link_libraries(TestSweepLineSpeed geos)
  endif()

  add_test(bug234 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bug234)
  add_test(TestSweepLineSpeed ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/TestSweepLineSpeed)

  set(STATUS_RESULT "ON")
endif()

message(STATUS "${STATUS_MESSAGE} - ${STATUS_RESULT}")
