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

# test that std::filesystem header actually is there and works
if(NOT CMAKE_REQUIRED_QUIET)
  # CMake 3.17+ use CHECK_START/CHECK_PASS/CHECK_FAIL
  message(STATUS "Checking if compiler supports std::filesystem")
endif()
set(_filename "${CMAKE_CURRENT_BINARY_DIR}/has_filesystem.cpp")
write_file(${_filename} "\
#include <filesystem>
int main() {
  std::filesystem::path pth {\"../\"};
  return 0;
}")
try_compile(HAVE_STD_FILESYSTEM "${CMAKE_BINARY_DIR}/temp" "${_filename}")
file(REMOVE "${_filename}")
unset(_filename)
if(HAVE_STD_FILESYSTEM)
  if(NOT CMAKE_REQUIRED_QUIET)
    # CMake 3.17+ use CHECK_START/CHECK_PASS/CHECK_FAIL
    message(STATUS "Checking if compiler supports std::filesystem - yes")
  endif()
  target_compile_definitions(test_geos_unit PRIVATE HAVE_STD_FILESYSTEM)
else()
  if(NOT CMAKE_REQUIRED_QUIET)
    # CMake 3.17+ use CHECK_START/CHECK_PASS/CHECK_FAIL
    message(STATUS "Checking if compiler supports std::filesystem - no")
  endif()
  target_compile_definitions(test_geos_unit PRIVATE HAVE_STD_FILESYSTEM=0)
endif()

find_package(Threads)

target_link_libraries(test_geos_unit PRIVATE geos geos_c Threads::Threads)
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
    add_test(NAME unit-${_cmake_testname} COMMAND test_geos_unit ${_testname} "--data" "${CMAKE_CURRENT_LIST_DIR}/../resources")
    set_tests_properties(unit-${_cmake_testname} PROPERTIES TIMEOUT 30)
endforeach()

# Run all the unit tests in one go, for faster memory checking
# under valgrind. Restrict to one configuration so it is only
# run with 'ctest -C Valgrind'
# Exemplar commandline:
#
# ctest --output-on-failure \
#   --overwrite MemoryCheckCommandOptions="--leak-check=full --error-exitcode=100" \
#   -R all-unit-tests -C Valgrind -T memcheck
#
add_test(NAME all-unit-tests
    COMMAND test_geos_unit
    CONFIGURATIONS Valgrind
    )

unset(_cmake_testname)
unset(_testfile)
unset(_testfiles)
unset(_testname)

