#################################################################################
# $Id$
#
# GEOS C++ library build configuration for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation. 
# See the COPYING file for more information.
#
#################################################################################

file(GLOB_RECURSE geos_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE geos_ALL_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h) # fix source_group issue

add_library(geos SHARED ${geos_SOURCES} ${geos_ALL_HEADERS})
add_library(geos-static STATIC ${geos_SOURCES} ${geos_ALL_HEADERS})

# TODO: Enable SOVERSION property
set_target_properties(geos
  PROPERTIES
  DEFINE_SYMBOL GEOS_DLL_EXPORT
  VERSION ${VERSION}
  CLEAN_DIRECT_OUTPUT 1)

set_target_properties(geos-static
  PROPERTIES
  OUTPUT_NAME "geos"
  PREFIX "lib"
  CLEAN_DIRECT_OUTPUT 1)

#################################################################################
# Installation
#################################################################################

install(TARGETS geos geos-static
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

#################################################################################
# Group source files for IDE source explorers (e.g. Visual Studio)
#################################################################################

file(GLOB top_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h)
source_group("Header Files" FILES ${top_HEADERS})
file(GLOB geos_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/*.h)
source_group("Header Files\\geos" FILES ${geos_HEADERS})
### geos/algorithm
file(GLOB geos_algorithm_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/algorithm/*.h)
source_group("Header Files\\geos\\algorithm" FILES ${geos_algorithm_HEADERS})
file(GLOB geos_algorithm_distance_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/algorithm/distance/*.h)
source_group("Header Files\\geos\\algorithm\\distance" FILES ${geos_algorithm_distance_HEADERS})
file(GLOB geos_algorithm_locate_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/algorithm/locate/*.h)
source_group("Header Files\\geos\\algorithm\\locate" FILES ${geos_algorithm_locate_HEADERS})
### geos/geom
file(GLOB geos_geom_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/geom/*.h)
source_group("Header Files\\geos\\geom" FILES ${geos_geom_HEADERS})
file(GLOB geos_geom_prep_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/geom/prep/*.h)
source_group("Header Files\\geos\\geom\\prep" FILES ${geos_geom_prep_HEADERS})
file(GLOB geos_geom_util_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/geom/util/*.h)
source_group("Header Files\\geos\\geom\\util" FILES ${geos_geom_util_HEADERS})
### geos/geomgraph
file(GLOB geos_geomgraph_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/geomgraph/*.h)
source_group("Header Files\\geos\\geomgraph" FILES ${geos_geomgraph_HEADERS})
file(GLOB geos_geomgraph_index_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/geomgraph/index/*.h)
source_group("Header Files\\geos\\geomgraph\\index" FILES ${geos_geomgraph_index_HEADERS})
### geos/index
file(GLOB geos_index_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/index/*.h)
source_group("Header Files\\geos\\index" FILES ${geos_index_HEADERS})
file(GLOB geos_index_bintree_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/index/bintree/*.h)
source_group("Header Files\\geos\\index\\bintree" FILES ${geos_index_bintree_HEADERS})
file(GLOB geos_index_chain_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/index/chain/*.h)
source_group("Header Files\\geos\\index\\chain" FILES ${geos_index_chain_HEADERS})
file(GLOB geos_index_intervalrtree_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/index/intervalrtree/*.h)
source_group("Header Files\\geos\\index\\intervalrtree" FILES ${geos_index_intervalrtree_HEADERS})
file(GLOB geos_index_quadtree_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/index/quadtree/*.h)
source_group("Header Files\\geos\\index\\quadtree" FILES ${geos_index_quadtree_HEADERS})
file(GLOB geos_index_strtree_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/index/strtree/*.h)
source_group("Header Files\\geos\\index\\strtree" FILES ${geos_index_strtree_HEADERS})
file(GLOB geos_index_sweepline_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/index/sweepline/*.h)
source_group("Header Files\\geos\\index\\sweepline" FILES ${geos_index_sweepline_HEADERS})
### geos/io
file(GLOB geos_io_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/io/*.h)
source_group("Header Files\\geos\\io" FILES ${geos_io_HEADERS})
### geos/linearref
file(GLOB geos_linearref_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/linearref/*.h)
source_group("Header Files\\geos\\linearref" FILES ${geos_linearref_HEADERS})
### geos/noding
file(GLOB geos_noding_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/noding/*.h)
source_group("Header Files\\geos\\noding" FILES ${geos_noding_HEADERS})
file(GLOB geos_noding_snapround_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/noding/snapround/*.h)
source_group("Header Files\\geos\\noding\\snapround" FILES ${geos_noding_snapround_HEADERS})
### geos/operation 
file(GLOB geos_operation_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/*.h)
source_group("Header Files\\geos\\operation" FILES ${geos_operation_HEADERS})
file(GLOB geos_operation_buffer_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/buffer/*.h)
source_group("Header Files\\geos\\operation\\buffer" FILES ${geos_operation_buffer_HEADERS})
file(GLOB geos_operation_distance_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/distance/*.h)
source_group("Header Files\\geos\\operation\\distance" FILES ${geos_operation_distance_HEADERS})
file(GLOB geos_operation_linemerge_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/linemerge/*.h)
source_group("Header Files\\geos\\operation\\linemerge" FILES ${geos_operation_linemerge_HEADERS})
file(GLOB geos_operation_overlay_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/overlay/*.h)
source_group("Header Files\\geos\\operation\\overlay" FILES ${geos_operation_overlay_HEADERS})
file(GLOB geos_operation_overlay_snap_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/overlay/snap/*.h)
source_group("Header Files\\geos\\operation\\overlay\\snap" FILES ${geos_operation_overlay_snap_HEADERS})
file(GLOB geos_operation_overlay_validate_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/overlay/validate/*.h)
source_group("Header Files\\geos\\operation\\overlay\\validate" FILES ${geos_operation_overlay_validate_HEADERS})
file(GLOB geos_operation_polygonize_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/polygonize/*.h)
source_group("Header Files\\geos\\operation\\polygonize" FILES ${geos_operation_polygonize_HEADERS})
file(GLOB geos_operation_predicate_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/predicate/*.h)
source_group("Header Files\\geos\\operation\\predicate" FILES ${geos_operation_predicate_HEADERS})
file(GLOB geos_operation_relate_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/relate/*.h)
source_group("Header Files\\geos\\operation\\relate" FILES ${geos_operation_relate_HEADERS})
file(GLOB geos_operation_sharedpaths_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/sharedpaths/*.h)
source_group("Header Files\\geos\\operation\\sharedpaths" FILES ${geos_operation_sharedpaths_HEADERS})
file(GLOB geos_operation_union_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/union/*.h)
source_group("Header Files\\geos\\operation\\union" FILES ${geos_operation_union_HEADERS})
file(GLOB geos_operation_union_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/union/*.h)
source_group("Header Files\\geos\\operation\\union" FILES ${geos_operation_union_HEADERS})
file(GLOB geos_operation_valid_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/operation/valid/*.h)
source_group("Header Files\\geos\\operation\\valid" FILES ${geos_operation_valid_HEADERS})
### geos/planargraph
file(GLOB geos_planargraph_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/planargraph/*.h)
source_group("Header Files\\geos\\planargraph" FILES ${geos_planargraph_HEADERS})
file(GLOB geos_planargraph_algorithm_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/planargraph/algorithm/*.h)
source_group("Header Files\\geos\\planargraph\\algorithm" FILES ${geos_planargraph_algorithm_HEADERS})
### geso/precision
file(GLOB geos_precision_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/precision/*.h)
source_group("Header Files\\geos\\precision" FILES ${geos_precision_HEADERS})
### geso/simplify
file(GLOB geos_simplify_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/simplify/*.h)
source_group("Header Files\\geos\\simplify" FILES ${geos_simplify_HEADERS})
### geso/util
file(GLOB geos_util_HEADERS ${CMAKE_SOURCE_DIR}/include/geos/util/*.h)
source_group("Header Files\\geos\\util" FILES ${geos_util_HEADERS})


