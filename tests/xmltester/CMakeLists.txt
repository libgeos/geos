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

add_executable(test_simplewkttester SimpleWKTTester.cpp)
target_link_libraries(test_simplewkttester PRIVATE geos)

add_executable(test_xmltester
    XMLTester.cpp
    BufferResultMatcher.cpp
    SingleSidedBufferResultMatcher.cpp
    tinyxml2/tinyxml2.h
    tinyxml2/tinyxml2.cpp)
target_link_libraries(test_xmltester PRIVATE geos)
target_include_directories(test_xmltester
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/tinyxml)

foreach(_testdir general issue misc robust validate)
    file(GLOB_RECURSE _testfiles ${CMAKE_CURRENT_LIST_DIR}/tests/${_testdir}/*.xml CONFIGURE_DEPEND)
    foreach(_testfile ${_testfiles})
        get_filename_component(_testname ${_testfile} NAME_WE)
        add_test(NAME ${_testdir}-${_testname} COMMAND test_xmltester -v --test-valid-output ${_testfile})
    endforeach()
endforeach()
unset(_testdir)
unset(_testname)
unset(_testfile)
unset(_testfiles)
