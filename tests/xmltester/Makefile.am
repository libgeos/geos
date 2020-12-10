#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
#prefix=@prefix@
#top_srcdir=@top_srcdir@
#top_builddir=@top_builddir@

AUTOMAKE_OPTIONS = subdir-objects

TESTS = testrunner

CLEANFILES = testrunner

EXTRA_DIST = testrunner.sh CMakeLists.txt

SAFE_XMLTESTS= \
	$(srcdir)/tests/general/TestBoundary.xml \
	$(srcdir)/tests/general/TestBuffer.xml \
	$(srcdir)/tests/general/TestBufferMitredJoin.xml \
	$(srcdir)/tests/general/TestCentroid.xml \
	$(srcdir)/tests/general/TestConvexHull-big.xml \
	$(srcdir)/tests/general/TestConvexHull.xml \
	$(srcdir)/tests/general/TestDistance.xml \
	$(srcdir)/tests/general/TestDensify.xml \
	$(srcdir)/tests/general/TestEqualsExact.xml \
	$(srcdir)/tests/general/TestInteriorPoint.xml \
	$(srcdir)/tests/general/TestIntersectsPL.xml \
	$(srcdir)/tests/general/TestMinimumClearance.xml \
	$(srcdir)/tests/general/TestNGOverlayA.xml \
	$(srcdir)/tests/general/TestNGOverlayAPrec.xml \
	$(srcdir)/tests/general/TestNGOverlayEmpty.xml \
	$(srcdir)/tests/general/TestNGOverlayL.xml \
	$(srcdir)/tests/general/TestNGOverlayLPrec.xml \
	$(srcdir)/tests/general/TestNGOverlayP.xml \
	$(srcdir)/tests/general/TestNGOverlayPPrec.xml \
	$(srcdir)/tests/general/TestOverlayAA.xml \
	$(srcdir)/tests/general/TestOverlayPLPrec.xml \
	$(srcdir)/tests/general/TestOverlayLLPrec.xml \
	$(srcdir)/tests/general/TestOverlayLL.xml \
	$(srcdir)/tests/general/TestOverlayLA.xml \
	$(srcdir)/tests/general/TestOverlayAAPrec.xml \
	$(srcdir)/tests/general/TestOverlayPL.xml \
	$(srcdir)/tests/general/TestOverlayPA.xml \
	$(srcdir)/tests/general/TestOverlayEmpty.xml \
	$(srcdir)/tests/general/TestOverlayPP.xml \
	$(srcdir)/tests/general/TestOverlayLAPrec.xml \
	$(srcdir)/tests/general/TestPreparedIntersectsPL.xml \
	$(srcdir)/tests/general/TestPreparedPointPredicate.xml \
	$(srcdir)/tests/general/TestPreparedPolygonPredicate.xml \
	$(srcdir)/tests/general/TestPreparedPredicatesWithGeometryCollection.xml \
	$(srcdir)/tests/general/TestRectanglePredicate.xml \
	$(srcdir)/tests/general/TestRelateAA.xml \
	$(srcdir)/tests/general/TestRelateAC.xml \
	$(srcdir)/tests/general/TestRelateLA.xml \
	$(srcdir)/tests/general/TestRelateLC.xml \
	$(srcdir)/tests/general/TestRelateLL.xml \
	$(srcdir)/tests/general/TestRelatePA.xml \
	$(srcdir)/tests/general/TestRelatePL.xml \
	$(srcdir)/tests/general/TestRelatePP.xml \
	$(srcdir)/tests/general/TestSimple.xml \
	$(srcdir)/tests/general/TestUnaryUnion.xml \
	$(srcdir)/tests/general/TestUnaryUnionFloating.xml \
	$(srcdir)/tests/general/TestValid.xml \
	$(srcdir)/tests/general/TestValid2-big.xml \
	$(srcdir)/tests/general/TestValid2.xml \
	$(srcdir)/tests/general/TestWithinDistance.xml \
	$(srcdir)/tests/misc/Buffer-1.xml \
	$(srcdir)/tests/misc/Buffer-2.xml \
	$(srcdir)/tests/misc/InvalidRelates.xml \
	$(srcdir)/tests/misc/Segfaults.xml \
	$(srcdir)/tests/misc/TestBufferExternal-1.xml \
	$(srcdir)/tests/misc/TestBufferExternal-2.xml \
	$(srcdir)/tests/misc/TestIsValid.xml \
	$(srcdir)/tests/misc/buildarea.xml \
	$(srcdir)/tests/misc/fme.xml \
	$(srcdir)/tests/misc/heisenbugs.xml \
	$(srcdir)/tests/misc/hexwkb.xml \
	$(srcdir)/tests/misc/hole_from_shell.xml \
	$(srcdir)/tests/misc/hole_red.xml \
	$(srcdir)/tests/misc/linemerge.xml \
	$(srcdir)/tests/misc/makevalid.xml \
	$(srcdir)/tests/misc/robustness.xml \
	$(srcdir)/tests/misc/safe-16595.xml \
	$(srcdir)/tests/misc/safe-16596.xml \
	$(srcdir)/tests/misc/safe-TestBufferJagged.xml \
	$(srcdir)/tests/misc/singlesidedbuffer.xml \
	$(srcdir)/tests/misc/stmlf-20061020-invalid-output.xml \
	$(srcdir)/tests/misc/stmlf-20061020.xml \
	$(srcdir)/tests/misc/stmlf-20070119.xml \
	$(srcdir)/tests/misc/split.xml \
	$(srcdir)/tests/robust/TestRobustOverlayFixed.xml \
	$(srcdir)/tests/robust/TestRobustRelate.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-misc-5.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-1034.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-153.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-234.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-275.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-350.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-358.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-360.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-368.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-392-lines.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-392.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-398.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-459.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-488.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-522.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-527.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-586.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-599.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-600-lines.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-615.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-737.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-837.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-838.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-979.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-994.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-997-union-fail.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-997-union-slow.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-1046-union-lines.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-1051.xml
	$(srcdir)/tests/robust/overlay/TestOverlay-geos-list.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-gisse-345341-lines.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-isochrone.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-jts-300.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-misc-1.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-misc-2.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-misc-3.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-misc-4.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-misc-5.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-osmwater.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-pg-4182-2.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-pg-4538.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-pg-list.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-qgis-29400.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-qgis-31552.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-qgis-37032.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-rsf-794.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-shapely-829.xml \
	$(srcdir)/tests/robust/overlay/TestOverlay-stmlf.xml \
	$(srcdir)/tests/issue/issue-geos-176.xml \
	$(srcdir)/tests/issue/issue-geos-188.xml \
	$(srcdir)/tests/issue/issue-geos-234.xml \
	$(srcdir)/tests/issue/issue-geos-244.xml \
	$(srcdir)/tests/issue/issue-geos-275.xml \
	$(srcdir)/tests/issue/issue-geos-350.xml \
	$(srcdir)/tests/issue/issue-geos-356.xml \
	$(srcdir)/tests/issue/issue-geos-358.xml \
	$(srcdir)/tests/issue/issue-geos-360.xml \
	$(srcdir)/tests/issue/issue-geos-366.xml \
	$(srcdir)/tests/issue/issue-geos-392.xml \
	$(srcdir)/tests/issue/issue-geos-398.xml \
	$(srcdir)/tests/issue/issue-geos-434.xml \
	$(srcdir)/tests/issue/issue-geos-459.xml \
	$(srcdir)/tests/issue/issue-geos-488.xml \
	$(srcdir)/tests/issue/issue-geos-527.xml \
	$(srcdir)/tests/issue/issue-geos-569.xml \
	$(srcdir)/tests/issue/issue-geos-582.xml \
	$(srcdir)/tests/issue/issue-geos-586.xml \
	$(srcdir)/tests/issue/issue-geos-599.xml \
	$(srcdir)/tests/issue/issue-geos-605.xml \
	$(srcdir)/tests/issue/issue-geos-615.xml \
	$(srcdir)/tests/issue/issue-geos-716.xml \
	$(srcdir)/tests/issue/issue-geos-837.xml \
	$(srcdir)/tests/issue/issue-geos-838.xml \
	$(srcdir)/tests/issue/issue-geos-990.xml \
	$(srcdir)/tests/issue/issue-geos-994.xml \
	$(srcdir)/tests/issue/issue-geos-1018.xml \
	$(srcdir)/tests/issue/issue-geos-1085.xml \
	$(srcdir)/tests/validate/TestRelateAA-big.xml \
	$(srcdir)/tests/validate/TestRelateAA.xml \
	$(srcdir)/tests/validate/TestRelateAC.xml \
	$(srcdir)/tests/validate/TestRelateLA.xml \
	$(srcdir)/tests/validate/TestRelateLC.xml \
	$(srcdir)/tests/validate/TestRelateLL.xml \
	$(srcdir)/tests/validate/TestRelatePA.xml \
	$(srcdir)/tests/validate/TestRelatePL.xml \
	$(srcdir)/tests/validate/TestRelatePP.xml

INVALID_OUTPUT_XMLTESTS =

FAILING_XMLTESTS = \
	$(srcdir)/tests/failure/robust-overlay-stmlf-failures.xml \
	$(srcdir)/tests/failure/issue-geos-344.xml \
	$(srcdir)/tests/failure/misc-TestBigNastyBuffer.xml \
	$(srcdir)/tests/failure/misc-TestSameDirection.xml \
	$(srcdir)/tests/failure/robust-TestRobustRelateFloat.xml


XMLTESTS=$(SAFE_XMLTESTS) $(INVALID_OUTPUT_XMLTESTS) $(FAILING_XMLTESTS)

EXTRA_DIST += $(XMLTESTS)

noinst_PROGRAMS = SimpleWKTTester XMLTester

LIBS = $(top_builddir)/src/libgeos.la

SimpleWKTTester_SOURCES = SimpleWKTTester.cpp
SimpleWKTTester_LDADD = $(LIBS)

XMLTester_SOURCES = \
	XMLTester.cpp \
	XMLTester.h \
	tinyxml2/tinyxml2.h \
	tinyxml2/tinyxml2.cpp \
	BufferResultMatcher.h \
	BufferResultMatcher.cpp \
	SingleSidedBufferResultMatcher.h \
	SingleSidedBufferResultMatcher.cpp

XMLTester_LDADD = $(LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_srcdir)/src/io/tinyxml -DTIXML_USE_STL

static:
	$(CXX) $(CXXFLAGS) -I../include -o staticXMLTester XMLTester.cpp MarkupSTL.o ../../src/geom/.libs/libgeos.a

testrunner: testrunner.sh Makefile
	@sh $(srcdir)/testrunner.sh $@ --test-valid-output $(SAFE_XMLTESTS)

