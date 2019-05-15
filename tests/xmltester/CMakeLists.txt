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

add_executable(test_simplewkttester SimpleWKTTester.cpp)
target_link_libraries(test_simplewkttester PRIVATE geos)

add_executable(test_xmltester)
target_sources(test_xmltester
  PRIVATE
    XMLTester.cpp
    BufferResultMatcher.cpp
    SingleSidedBufferResultMatcher.cpp
    tinyxml2/tinyxml2.h
    tinyxml2/tinyxml2.cpp)
target_link_libraries(test_xmltester PRIVATE geos)
target_include_directories(test_xmltester
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/tinyxml)

file(GLOB_RECURSE _tests ${CMAKE_CURRENT_LIST_DIR}/tests/*.xml CONFIGURE_DEPEND)
list(FILTER _tests EXCLUDE REGEX failure)
add_test(NAME test_xmltester COMMAND test_xmltester --test-valid-output ${_tests})
unset(_tests)
