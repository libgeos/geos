#################################################################################
#
# CMake configuration for GEOS perf tests
#
# Copyright (C) 2017 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#
#################################################################################

set(STATUS_MESSAGE "Enable GEOS performance tests build")
set(STATUS_RESULT "OFF")

if(GEOS_ENABLE_TESTS)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  add_executable(perf_class_sizes ClassSizes.cpp)

  target_link_libraries(perf_class_sizes geos)

  add_test(perf_class_sizes
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/perf_class_sizes)

  add_subdirectory(operation)
  add_subdirectory(capi)

  set(STATUS_RESULT "ON")
endif()

message(STATUS "${STATUS_MESSAGE} - ${STATUS_RESULT}")
