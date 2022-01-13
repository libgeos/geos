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
    add_executable(perf_monotone_chain MonotoneChainPerfTest.cpp)
    target_include_directories(perf_monotone_chain PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
    target_link_libraries(perf_monotone_chain PRIVATE
            benchmark::benchmark geos)

    add_executable(perf_monotone_chain_builder MonotoneChainBuilderPerfTest.cpp)
    target_include_directories(perf_monotone_chain_builder PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
    target_link_libraries(perf_monotone_chain_builder PRIVATE
            benchmark::benchmark geos)
endif()
