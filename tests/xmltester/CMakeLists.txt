#################################################################################
# $Id: CMakeLists.txt 3382 2011-05-30 09:41:41Z mloskot $
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

set(STATUS_MESSAGE "Enable GEOS XML Tester build")
set(STATUS_RESULT "OFF")

if(GEOS_ENABLE_TESTS)

  include_directories(${CMAKE_CURRENT_SOURCE_DIR})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tinyxml)

  set(simplewkttester_SOURCES SimpleWKTTester.cpp)

  set(xmltester_SOURCES
    XMLTester.cpp
    tinyxml/tinyxml.cpp
    tinyxml/tinystr.cpp
    tinyxml/tinyxmlerror.cpp
    tinyxml/tinyxmlparser.cpp
    BufferResultMatcher.cpp
    SingleSidedBufferResultMatcher.cpp)

  add_executable(simplewkttester ${simplewkttester_SOURCES})
  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
    target_link_libraries(simplewkttester GEOS)
 	else()
    target_link_libraries(simplewkttester geos)
 	endif()

  add_executable(xmltester ${xmltester_SOURCES})
  if(APPLE AND GEOS_ENABLE_MACOSX_FRAMEWORK)
    target_link_libraries(xmltester GEOS)
 	else()
    target_link_libraries(xmltester geos)
  endif()

  set(XMLTESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
  set(SAVE_XMLTESTS
    ${XMLTESTS_DIR}/testLeaksBig.xml
    ${XMLTESTS_DIR}/split.xml
    ${XMLTESTS_DIR}/hexwkb.xml
    ${XMLTESTS_DIR}/test.xml
    ${XMLTESTS_DIR}/linemerge.xml
    ${XMLTESTS_DIR}/TestIsValid.xml
    ${XMLTESTS_DIR}/robustness.xml
    ${XMLTESTS_DIR}/buffer.xml
    ${XMLTESTS_DIR}/test.xml
    ${XMLTESTS_DIR}/singlesidedbuffer.xml
    ${XMLTESTS_DIR}/bug176.xml
    ${XMLTESTS_DIR}/bug188.xml
    ${XMLTESTS_DIR}/bug244.xml
    ${XMLTESTS_DIR}/bug275.xml
    ${XMLTESTS_DIR}/general/TestBoundary.xml
    ${XMLTESTS_DIR}/general/TestBuffer.xml
    ${XMLTESTS_DIR}/general/TestBufferMitredJoin.xml
    ${XMLTESTS_DIR}/general/TestCentroid.xml
    ${XMLTESTS_DIR}/general/TestConvexHull.xml
    ${XMLTESTS_DIR}/general/TestConvexHull-big.xml
    ${XMLTESTS_DIR}/general/TestFunctionAAPrec.xml
    ${XMLTESTS_DIR}/general/TestFunctionAA.xml
    ${XMLTESTS_DIR}/general/TestFunctionLAPrec.xml
    ${XMLTESTS_DIR}/general/TestFunctionLA.xml
    ${XMLTESTS_DIR}/general/TestFunctionLLPrec.xml
    ${XMLTESTS_DIR}/general/TestFunctionLL.xml
    ${XMLTESTS_DIR}/general/TestFunctionPA.xml
    ${XMLTESTS_DIR}/general/TestFunctionPLPrec.xml
    ${XMLTESTS_DIR}/general/TestFunctionPL.xml
    ${XMLTESTS_DIR}/general/TestFunctionPP.xml
    ${XMLTESTS_DIR}/general/TestInteriorPoint.xml
    ${XMLTESTS_DIR}/general/TestRectanglePredicate.xml
    ${XMLTESTS_DIR}/general/TestRelateAA.xml
    ${XMLTESTS_DIR}/general/TestRelateLA.xml
    ${XMLTESTS_DIR}/general/TestRelateLL.xml
    ${XMLTESTS_DIR}/general/TestRelatePL.xml
    ${XMLTESTS_DIR}/general/TestRelateAC.xml
    ${XMLTESTS_DIR}/general/TestRelateLC.xml
    ${XMLTESTS_DIR}/general/TestRelatePA.xml
    ${XMLTESTS_DIR}/general/TestRelatePP.xml
    ${XMLTESTS_DIR}/general/TestSimple.xml
    ${XMLTESTS_DIR}/general/TestValid.xml
    ${XMLTESTS_DIR}/general/TestValid2.xml
    ${XMLTESTS_DIR}/general/TestValid2-big.xml
    ${XMLTESTS_DIR}/general/TestWithinDistance.xml
    ${XMLTESTS_DIR}/stmlf/stmlf-cases-20061020.xml
    ${XMLTESTS_DIR}/stmlf/stmlf-cases-20070119.xml
    ${XMLTESTS_DIR}/robust/TestRobustOverlayFixed.xml
    ${XMLTESTS_DIR}/robust/TestRobustRelate.xml
    ${XMLTESTS_DIR}/fme.xml
    ${XMLTESTS_DIR}/TestBufferExternal.xml
    ${XMLTESTS_DIR}/TestBufferExternal2.xml
    ${XMLTESTS_DIR}/heisenbugs.xml
    ${XMLTESTS_DIR}/badguy3.xml
    ${XMLTESTS_DIR}/hole_from_shell.xml
    ${XMLTESTS_DIR}/hole_red.xml
    ${XMLTESTS_DIR}/safe/16595.xml
    ${XMLTESTS_DIR}/safe/16596.xml)

  add_test(xmltester ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/xmltester ${SAVE_XMLTESTS})

  set(STATUS_RESULT "ON")
endif()

message(STATUS "${STATUS_MESSAGE} - ${STATUS_RESULT}")
