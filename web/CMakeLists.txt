################################################################################
# Part of CMake configuration for GEOS
#
# Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
################################################################################


if(BUILD_WEBSITE)

    # Can't build website without hugo
    find_program(
        HUGO_EXE NAMES hugo
        REQUIRED
        )

    message(STATUS "GEOS: Build website ON")

    add_custom_target(web
        COMMAND ${HUGO_EXE}
            --source ${CMAKE_CURRENT_SOURCE_DIR}
            --destination ${CMAKE_CURRENT_BINARY_DIR}/html
        COMMENT "Building web site to ${CMAKE_CURRENT_BINARY_DIR}/html"
        )

endif()
