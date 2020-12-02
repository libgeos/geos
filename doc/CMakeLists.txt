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

add_executable(example example.cpp)

target_link_libraries(example PRIVATE geos)
if(BUILD_TESTING)
    target_include_directories(test_geos_unit
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
endif()

option(BUILD_DOCUMENTATION "Build Doxygen documentation" OFF)

if(BUILD_DOCUMENTATION)
    find_package(Doxygen)

    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen was not found.")
    endif()

    set(srcdir ${CMAKE_CURRENT_SOURCE_DIR})
    set(VERSION ${GEOS_VERSION})
    set(DOXYGEN_LOGFILE ${CMAKE_CURRENT_BINARY_DIR}/doxygen.log)
    set(CHECK_ERROR_SCRIPT "check_doxygen_errors.cmake")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/${CHECK_ERROR_SCRIPT}"
        "${CMAKE_CURRENT_BINARY_DIR}/${CHECK_ERROR_SCRIPT}"
        COPYONLY)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY)
    unset(srcdir)
    unset(VERSION)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        BYPRODUCTS ${DOXYGEN_LOGFILE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen_docs)

    add_test(NAME test_docs
        COMMAND ${CMAKE_COMMAND}
            -D DOXYGEN_LOGFILE="${DOXYGEN_LOGFILE}"
            -P "${CMAKE_CURRENT_BINARY_DIR}/${CHECK_ERROR_SCRIPT}")

    unset(DOXYGEN_LOGFILE)
    unset(CHECK_ERROR_SCRIPT)
endif()
