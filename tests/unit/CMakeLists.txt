################################################################################
# Part of CMake configuration for GEOS
#
# Copyright (C) 2018 Mateusz Loskot <mateusz@loskot.net>
# Copyright (C) 2019 Daniel Baston <dbaston@gmail.com>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
################################################################################
file(GLOB_RECURSE _sources ${CMAKE_CURRENT_LIST_DIR}/*.cpp CONFIGURE_DEPEND)
add_executable(test_geos_unit ${_sources})
unset(_sources)

target_link_libraries(test_geos_unit PRIVATE geos geos_c)
target_include_directories(test_geos_unit
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

file(GLOB_RECURSE _testfiles ${CMAKE_CURRENT_LIST_DIR}/**/*Test.cpp CONFIGURE_DEPEND)
foreach(_testfile ${_testfiles})
    string(REPLACE ${CMAKE_CURRENT_LIST_DIR}/ "" _testname ${_testfile})
    string(REPLACE "Test.cpp" "" _testname ${_testname})
    string(REPLACE "/" "-" _cmake_testname  ${_testname})
    string(REPLACE "/" "::" _testname ${_testname})
    if (NOT ${_testname} MATCHES "^capi::")
      string(CONCAT _testname "geos::" ${_testname})
    endif()
    add_test(NAME unit-${_cmake_testname} COMMAND test_geos_unit ${_testname})
endforeach()
unset(_cmake_testname)
unset(_testfile)
unset(_testfiles)
unset(_testname)

