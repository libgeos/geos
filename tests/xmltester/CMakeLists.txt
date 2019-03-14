#################################################################################
#
# GEOS XML tests runner build configuration for CMake build system
#
# Copyright (C) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# This is free software; you can redistribute and/or modify it under
# the terms of the GNU Lesser General Public Licence as published
# by the Free Software Foundation.
# See the COPYING file for more information.
#
#################################################################################

set(STATUS_MESSAGE "Enable GEOS XML tests build")
set(STATUS_RESULT "OFF")

if(GEOS_ENABLE_TESTS)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tinyxml2)

  set(simplewkttester_SOURCES SimpleWKTTester.cpp)

  set(xmltester_SOURCES
    XMLTester.cpp
    tinyxml2/tinyxml2.h
    tinyxml2/tinyxml2.cpp
    BufferResultMatcher.cpp
    SingleSidedBufferResultMatcher.cpp)

  add_executable(test_simplewkttester ${simplewkttester_SOURCES})

  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
    target_link_libraries(test_simplewkttester GEOS)
 	else()
    target_link_libraries(test_simplewkttester geos)
 	endif()

  add_executable(test_xmltester ${xmltester_SOURCES})

  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
    target_link_libraries(test_xmltester GEOS)
 	else()
    target_link_libraries(test_xmltester geos)
  endif()

  set(XMLTESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
  set(SAVE_XMLTESTS
    ${XMLTESTS_DIR}/general/TestBoundary.xml
    ${XMLTESTS_DIR}/general/TestBuffer.xml
    ${XMLTESTS_DIR}/general/TestBufferMitredJoin.xml
    ${XMLTESTS_DIR}/general/TestCentroid.xml
    ${XMLTESTS_DIR}/general/TestConvexHull-big.xml
    ${XMLTESTS_DIR}/general/TestConvexHull.xml
    ${XMLTESTS_DIR}/general/TestDensify.xml
    ${XMLTESTS_DIR}/general/TestDistance.xml
    ${XMLTESTS_DIR}/general/TestEqualsExact.xml
    ${XMLTESTS_DIR}/general/TestFunctionAA.xml
    ${XMLTESTS_DIR}/general/TestFunctionAAPrec.xml
    ${XMLTESTS_DIR}/general/TestFunctionLA.xml
    ${XMLTESTS_DIR}/general/TestFunctionLAPrec.xml
    ${XMLTESTS_DIR}/general/TestFunctionLL.xml
    ${XMLTESTS_DIR}/general/TestFunctionLLPrec.xml
    ${XMLTESTS_DIR}/general/TestFunctionPA.xml
    ${XMLTESTS_DIR}/general/TestFunctionPL.xml
    ${XMLTESTS_DIR}/general/TestFunctionPLPrec.xml
    ${XMLTESTS_DIR}/general/TestFunctionPP.xml
    ${XMLTESTS_DIR}/general/TestInteriorPoint.xml
    ${XMLTESTS_DIR}/general/TestMinimumClearance.xml
    ${XMLTESTS_DIR}/general/TestPreparedPointPredicate.xml
    ${XMLTESTS_DIR}/general/TestPreparedPolygonPredicate.xml
    ${XMLTESTS_DIR}/general/TestPreparedPredicatesWithGeometryCollection.xml
    ${XMLTESTS_DIR}/general/TestRectanglePredicate.xml
    ${XMLTESTS_DIR}/general/TestRelateAA.xml
    ${XMLTESTS_DIR}/general/TestRelateAC.xml
    ${XMLTESTS_DIR}/general/TestRelateLA.xml
    ${XMLTESTS_DIR}/general/TestRelateLC.xml
    ${XMLTESTS_DIR}/general/TestRelateLL.xml
    ${XMLTESTS_DIR}/general/TestRelatePA.xml
    ${XMLTESTS_DIR}/general/TestRelatePL.xml
    ${XMLTESTS_DIR}/general/TestRelatePP.xml
    ${XMLTESTS_DIR}/general/TestSimple.xml
    ${XMLTESTS_DIR}/general/TestUnaryUnion.xml
    ${XMLTESTS_DIR}/general/TestUnaryUnionFloating.xml
    ${XMLTESTS_DIR}/general/TestValid.xml
    ${XMLTESTS_DIR}/general/TestValid2-big.xml
    ${XMLTESTS_DIR}/general/TestValid2.xml
    ${XMLTESTS_DIR}/general/TestWithinDistance.xml
    ${XMLTESTS_DIR}/misc/TestBufferExternal.xml
    ${XMLTESTS_DIR}/misc/TestBufferExternal2.xml
    ${XMLTESTS_DIR}/misc/TestIsValid.xml
    ${XMLTESTS_DIR}/misc/badguy3.xml
    ${XMLTESTS_DIR}/misc/buildarea.xml
    ${XMLTESTS_DIR}/misc/buffer.xml
    ${XMLTESTS_DIR}/misc/fme.xml
    ${XMLTESTS_DIR}/misc/heisenbugs.xml
    ${XMLTESTS_DIR}/misc/hexwkb.xml
    ${XMLTESTS_DIR}/misc/hole_from_shell.xml
    ${XMLTESTS_DIR}/misc/hole_red.xml
    ${XMLTESTS_DIR}/misc/linemerge.xml
    ${XMLTESTS_DIR}/misc/makevalid.xml
    ${XMLTESTS_DIR}/misc/robustness.xml
    ${XMLTESTS_DIR}/misc/safe-16595.xml
    ${XMLTESTS_DIR}/misc/safe-16596.xml
    ${XMLTESTS_DIR}/misc/safe-TestBufferJagged.xml
    ${XMLTESTS_DIR}/misc/singlesidedbuffer.xml
    ${XMLTESTS_DIR}/misc/stmlf-cases-20061020-invalid-output.xml
    ${XMLTESTS_DIR}/misc/stmlf-cases-20061020.xml
    ${XMLTESTS_DIR}/misc/stmlf-cases-20070119.xml
    ${XMLTESTS_DIR}/misc/split.xml
    ${XMLTESTS_DIR}/misc/InvalidRelates.xml
    ${XMLTESTS_DIR}/misc/Segfaults.xml
    ${XMLTESTS_DIR}/robust/TestRobustOverlayFixed.xml
    ${XMLTESTS_DIR}/robust/TestRobustRelate.xml
    ${XMLTESTS_DIR}/issue/issue-geos-176.xml
    ${XMLTESTS_DIR}/issue/issue-geos-188.xml
    ${XMLTESTS_DIR}/issue/issue-geos-244.xml
    ${XMLTESTS_DIR}/issue/issue-geos-275.xml
    ${XMLTESTS_DIR}/issue/issue-geos-350.xml
    ${XMLTESTS_DIR}/issue/issue-geos-356.xml
    ${XMLTESTS_DIR}/issue/issue-geos-358.xml
    ${XMLTESTS_DIR}/issue/issue-geos-360.xml
    ${XMLTESTS_DIR}/issue/issue-geos-366.xml
    ${XMLTESTS_DIR}/issue/issue-geos-392.xml
    ${XMLTESTS_DIR}/issue/issue-geos-398.xml
    ${XMLTESTS_DIR}/issue/issue-geos-434.xml
    ${XMLTESTS_DIR}/issue/issue-geos-459.xml
    ${XMLTESTS_DIR}/issue/issue-geos-527.xml
    ${XMLTESTS_DIR}/issue/issue-geos-569.xml
    ${XMLTESTS_DIR}/issue/issue-geos-582.xml
    ${XMLTESTS_DIR}/issue/issue-geos-586.xml
    ${XMLTESTS_DIR}/issue/issue-geos-599.xml
    ${XMLTESTS_DIR}/issue/issue-geos-605.xml
    ${XMLTESTS_DIR}/issue/issue-geos-615.xml
    ${XMLTESTS_DIR}/issue/issue-geos-716.xml
    ${XMLTESTS_DIR}/issue/issue-geos-837.xml
    ${XMLTESTS_DIR}/issue/issue-geos-838.xml
    ${XMLTESTS_DIR}/validate/TestRelateAA-big.xml
    ${XMLTESTS_DIR}/validate/TestRelateAA.xml
    ${XMLTESTS_DIR}/validate/TestRelateAC.xml
    ${XMLTESTS_DIR}/validate/TestRelateLA.xml
    ${XMLTESTS_DIR}/validate/TestRelateLC.xml
    ${XMLTESTS_DIR}/validate/TestRelateLL.xml
    ${XMLTESTS_DIR}/validate/TestRelatePA.xml
    ${XMLTESTS_DIR}/validate/TestRelatePL.xml
    ${XMLTESTS_DIR}/validate/TestRelatePP.xml
    )

  set(FAILING_XMLTESTS
    ${XMLTESTS_DIR}/issue/issue-geos-488.xml
    ${XMLTESTS_DIR}/issue/issue-geos-344.xml
    ${XMLTESTS_DIR}/robust/TestRobustRelateFloat.xml
    ${XMLTESTS_DIR}/failure/TestOverlay.xml
    ${XMLTESTS_DIR}/misc/TestBigNastyBuffer.xml
    ${XMLTESTS_DIR}/misc/TestSameDirection.xml
    )

  add_test(test_xmltester ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_xmltester -v --test-valid-output ${SAVE_XMLTESTS})

  set(STATUS_RESULT "ON")
endif()

message(STATUS "${STATUS_MESSAGE} - ${STATUS_RESULT}")
