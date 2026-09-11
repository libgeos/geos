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
            benchmark::benchmark)
    target_compile_features(perf_envelope PRIVATE cxx_std_17)
    target_compile_options(perf_envelope PRIVATE ${geos_cxx_private_flags} ${geos_cxx_interface_flags})
    target_compile_definitions(perf_envelope PRIVATE ${geos_cxx_private_definitions} ${geos_cxx_interface_definitions})
endif()

IF(benchmark_FOUND)
    add_executable(perf_coordseq
            CoordinateSequencePerfTest.cpp
            ${PROJECT_SOURCE_DIR}/src/geom/CoordinateSequence.cpp
            ${PROJECT_SOURCE_DIR}/src/geom/Coordinate.cpp)
    target_include_directories(perf_coordseq PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)
    target_link_libraries(perf_coordseq PRIVATE
            benchmark::benchmark)
    target_compile_features(perf_coordseq PRIVATE cxx_std_17)
    target_compile_options(perf_coordseq PRIVATE ${geos_cxx_private_flags} ${geos_cxx_interface_flags})
    target_compile_definitions(perf_coordseq PRIVATE ${geos_cxx_private_definitions} ${geos_cxx_interface_definitions})
endif()

add_executable(perf_topo_predicate
    TopologyPredicatePerfTest.cpp)
target_include_directories(perf_topo_predicate PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/benchmarks>)
target_link_libraries(perf_topo_predicate PRIVATE geos)
