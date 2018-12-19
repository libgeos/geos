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
    ${XMLTESTS_DIR}/TestBufferExternal.xml
    ${XMLTESTS_DIR}/TestBufferExternal2.xml
    ${XMLTESTS_DIR}/TestIsValid.xml
    ${XMLTESTS_DIR}/badguy3.xml
    ${XMLTESTS_DIR}/buffer.xml
    ${XMLTESTS_DIR}/fme.xml
    ${XMLTESTS_DIR}/heisenbugs.xml
    ${XMLTESTS_DIR}/hexwkb.xml
    ${XMLTESTS_DIR}/hole_from_shell.xml
    ${XMLTESTS_DIR}/hole_red.xml
    ${XMLTESTS_DIR}/linemerge.xml
    ${XMLTESTS_DIR}/robustness.xml
    ${XMLTESTS_DIR}/singlesidedbuffer.xml
    ${XMLTESTS_DIR}/split.xml
    ${XMLTESTS_DIR}/test.xml
    ${XMLTESTS_DIR}/testLeaksBig.xml
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
    ${XMLTESTS_DIR}/robust/TestRobustOverlayFixed.xml
    ${XMLTESTS_DIR}/robust/TestRobustRelate.xml
    ${XMLTESTS_DIR}/safe/16595.xml
    ${XMLTESTS_DIR}/safe/16596.xml
    ${XMLTESTS_DIR}/safe/TestBufferJagged.xml
    ${XMLTESTS_DIR}/stmlf/stmlf-cases-20061020.xml
    ${XMLTESTS_DIR}/stmlf/stmlf-cases-20070119.xml
    ${XMLTESTS_DIR}/ticket/bug176.xml
    ${XMLTESTS_DIR}/ticket/bug188.xml
    ${XMLTESTS_DIR}/ticket/bug244.xml
    ${XMLTESTS_DIR}/ticket/bug275.xml
    ${XMLTESTS_DIR}/ticket/bug350.xml
    ${XMLTESTS_DIR}/ticket/bug356.xml
    ${XMLTESTS_DIR}/ticket/bug358.xml
    ${XMLTESTS_DIR}/ticket/bug360.xml
    ${XMLTESTS_DIR}/ticket/bug366.xml
    ${XMLTESTS_DIR}/ticket/bug392.xml
    ${XMLTESTS_DIR}/ticket/bug398.xml
    ${XMLTESTS_DIR}/ticket/bug434.xml
    ${XMLTESTS_DIR}/ticket/bug459.xml
    ${XMLTESTS_DIR}/ticket/bug527.xml
    ${XMLTESTS_DIR}/ticket/bug569.xml
    ${XMLTESTS_DIR}/ticket/bug582.xml
    ${XMLTESTS_DIR}/ticket/bug586.xml
    ${XMLTESTS_DIR}/ticket/bug599.xml
    ${XMLTESTS_DIR}/ticket/bug605.xml
    ${XMLTESTS_DIR}/ticket/bug615.xml
    ${XMLTESTS_DIR}/ticket/bug716.xml
    ${XMLTESTS_DIR}/ticket/bug837.xml
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
    ${XMLTESTS_DIR}/ticket/bug488.xml
    ${XMLTESTS_DIR}/ticket/bug344.xml
    ${XMLTESTS_DIR}/robust/TestRobustRelateFloat.xml
    ${XMLTESTS_DIR}/TestBigNastyBuffer.xml
    ${XMLTESTS_DIR}/failure/TestOverlay.xml
    )

  add_test(test_xmltester ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_xmltester -v --test-valid-output ${SAVE_XMLTESTS})

  set(STATUS_RESULT "ON")
endif()

message(STATUS "${STATUS_MESSAGE} - ${STATUS_RESULT}")
