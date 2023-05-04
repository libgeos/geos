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

# Setup tinyxml as a separate library so we can avoid picking up GEOS compile flags
add_library(tinyxml2 STATIC
        tinyxml2/tinyxml2.h
        tinyxml2/tinyxml2.cpp)
target_compile_options(
        tinyxml2 PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wno-conversion>
        $<$<CXX_COMPILER_ID:Clang>:-Wno-conversion>
)

add_executable(test_xmltester
    XMLTester.cpp
    BufferResultMatcher.cpp
    SingleSidedBufferResultMatcher.cpp)
target_link_libraries(test_xmltester PRIVATE geos tinyxml2)
target_include_directories(test_xmltester
  SYSTEM PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/tinyxml2)

# Scan the directories of XML tests and create a test for each.
foreach(_testdir general issue misc robust validate)
    file(GLOB_RECURSE _testfiles
        ${CMAKE_CURRENT_LIST_DIR}/tests/${_testdir}/*.xml
        CONFIGURE_DEPEND)
    foreach(_testfile ${_testfiles})
        get_filename_component(_testname ${_testfile} NAME_WE)
        add_test(NAME xml-${_testdir}-${_testname}
            COMMAND test_xmltester -v --test-valid-output ${_testfile})
        list(APPEND _xmltests ${_testfile})
    endforeach()
endforeach()

# Run all the XML tests in one go, for faster memory checking
# under valgrind. Restrict to one configuration so it is only
# run with 'ctest -C Valgrind'
add_test(NAME all-xml-tests
    COMMAND test_xmltester -v --test-valid-output ${_xmltests}
    CONFIGURATIONS Valgrind
    )
unset(_xmltests)

# Disable tests that are known to fail on 32-bit systems.
set_tests_properties(xml-issue-issue-geos-837 PROPERTIES DISABLED $<EQUAL:4,${CMAKE_SIZEOF_VOID_P}>)

unset(_testdir)
unset(_testname)
unset(_testfile)
unset(_testfiles)
