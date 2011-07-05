#prefix=@prefix@
#top_srcdir=@top_srcdir@
#top_builddir=@top_builddir@

TESTS = testrunner

CLEANFILES = testrunner

EXTRA_DIST = testrunner.sh

SAFE_XMLTESTS=$(srcdir)/tests/testLeaksBig.xml \
	$(srcdir)/tests/split.xml \
	$(srcdir)/tests/hexwkb.xml \
	$(srcdir)/tests/test.xml \
	$(srcdir)/tests/linemerge.xml \
	$(srcdir)/tests/TestIsValid.xml \
	$(srcdir)/tests/robustness.xml \
	$(srcdir)/tests/buffer.xml \
	$(srcdir)/tests/test.xml \
	$(srcdir)/tests/singlesidedbuffer.xml \
	$(srcdir)/tests/bug176.xml \
	$(srcdir)/tests/bug188.xml \
	$(srcdir)/tests/bug244.xml \
	$(srcdir)/tests/bug275.xml \
	$(srcdir)/tests/general/TestBoundary.xml \
	$(srcdir)/tests/general/TestBuffer.xml \
	$(srcdir)/tests/general/TestBufferMitredJoin.xml \
	$(srcdir)/tests/general/TestCentroid.xml \
	$(srcdir)/tests/general/TestConvexHull.xml \
	$(srcdir)/tests/general/TestConvexHull-big.xml \
	$(srcdir)/tests/general/TestFunctionAAPrec.xml \
	$(srcdir)/tests/general/TestFunctionAA.xml \
	$(srcdir)/tests/general/TestFunctionLAPrec.xml \
	$(srcdir)/tests/general/TestFunctionLA.xml \
	$(srcdir)/tests/general/TestFunctionLLPrec.xml \
	$(srcdir)/tests/general/TestFunctionLL.xml \
	$(srcdir)/tests/general/TestFunctionPA.xml \
	$(srcdir)/tests/general/TestFunctionPLPrec.xml \
	$(srcdir)/tests/general/TestFunctionPL.xml \
	$(srcdir)/tests/general/TestFunctionPP.xml \
	$(srcdir)/tests/general/TestInteriorPoint.xml \
	$(srcdir)/tests/general/TestRectanglePredicate.xml \
	$(srcdir)/tests/general/TestRelateAA.xml \
	$(srcdir)/tests/general/TestRelateLA.xml \
	$(srcdir)/tests/general/TestRelateLL.xml \
	$(srcdir)/tests/general/TestRelatePL.xml \
	$(srcdir)/tests/general/TestRelateAC.xml \
	$(srcdir)/tests/general/TestRelateLC.xml \
	$(srcdir)/tests/general/TestRelatePA.xml \
	$(srcdir)/tests/general/TestRelatePP.xml \
	$(srcdir)/tests/general/TestSimple.xml \
	$(srcdir)/tests/general/TestValid.xml \
	$(srcdir)/tests/general/TestValid2.xml \
	$(srcdir)/tests/general/TestValid2-big.xml \
	$(srcdir)/tests/general/TestWithinDistance.xml \
	$(srcdir)/tests/stmlf/stmlf-cases-20061020.xml \
	$(srcdir)/tests/stmlf/stmlf-cases-20070119.xml \
	$(srcdir)/tests/robust/TestRobustOverlayFixed.xml \
	$(srcdir)/tests/robust/TestRobustRelate.xml \
	$(srcdir)/tests/fme.xml \
	$(srcdir)/tests/TestBufferExternal.xml \
	$(srcdir)/tests/TestBufferExternal2.xml \
	$(srcdir)/tests/heisenbugs.xml \
	$(srcdir)/tests/badguy3.xml \
	$(srcdir)/tests/hole_from_shell.xml \
	$(srcdir)/tests/hole_red.xml \
	$(srcdir)/tests/safe/16595.xml \
	$(srcdir)/tests/safe/16596.xml


XMLTESTS=$(SAFE_XMLTESTS) 

EXTRA_DIST += $(XMLTESTS)

noinst_PROGRAMS = SimpleWKTTester XMLTester
#bin_PROGRAMS = XMLTester
# CTS

LIBS = $(top_builddir)/source/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

# CTS_SOURCES = CTS.cpp
# CTS_LDADD = $(LIBS)

SimpleWKTTester_SOURCES = SimpleWKTTester.cpp
SimpleWKTTester_LDADD = $(LIBS)

XMLTester_SOURCES = XMLTester.cpp XMLTester.h \
	tinyxml/tinyxml.h tinyxml/tinyxml.cpp \
	tinyxml/tinystr.h tinyxml/tinystr.cpp \
	tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp \
	BufferResultMatcher.h BufferResultMatcher.cpp \
	SingleSidedBufferResultMatcher.h SingleSidedBufferResultMatcher.cpp

XMLTester_LDADD = $(LIBS)

INCLUDES = -I$(top_srcdir)/source/headers
INCLUDES += -I$(top_srcdir)/source/io/tinyxml -DTIXML_USE_STL

static:
	$(CXX) $(CXXFLAGS) -I../headers -o staticXMLTester XMLTester.cpp MarkupSTL.o ../../source/geom/.libs/libgeos.a

test: XMLTester
	./XMLTester $(XMLTESTS) 

testrunner: testrunner.sh Makefile
	sh $(srcdir)/testrunner.sh $@ $(SAFE_XMLTESTS)

verbose-test: XMLTester
	./XMLTester -v $(XMLTESTS) 
