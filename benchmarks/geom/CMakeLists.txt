################################################################################
# Part of CMake configuration for GEOS
#
# Copyright (C) 2021 Daniel Baston
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
################################################################################

IF(benchmark_FOUND)
    add_executable(perf_envelope
            EnvelopePerfTest.cpp
            ${PROJECT_SOURCE_DIR}/src/geom/Envelope.cpp)
    target_include_directories(perf_envelope PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
    target_link_libraries(perf_envelope PRIVATE
            benchmark::benchmark geos_cxx_flags)
endif()
