#prefix=@prefix@
#top_srcdir=@top_srcdir@
#top_builddir=@top_builddir@

TESTS = testrunner

CLEANFILES = testrunner

EXTRA_DIST = testrunner.sh

SAFE_XMLTESTS=$(srcdir)/tests/testLeaksBig.xml \
	$(srcdir)/split.xml \
	$(srcdir)/tests/TestSimple.xml \
	$(srcdir)/tests/hexwkb.xml \
	$(srcdir)/tests/test.xml \
	$(srcdir)/tests/linemerge.xml \
	$(srcdir)/tests/TestInteriorPoint.xml \
	$(srcdir)/tests/TestCentroid.xml \
	$(srcdir)/tests/TestIsValid.xml \
	$(srcdir)/tests/TestValid.xml \
	$(srcdir)/tests/TestValid2.xml \
	$(srcdir)/tests/TestValid2-big.xml \
	$(srcdir)/tests/robustness.xml \
	$(srcdir)/tests/stmlf/stmlf-cases-20061020.xml \
	$(srcdir)/tests/stmlf/stmlf-cases-20070119.xml \
	$(srcdir)/tests/robust/TestRobustOverlayFixed.xml \
	$(srcdir)/tests/robust/TestRobustRelate.xml \
	$(srcdir)/tests/fme.xml \
	$(srcdir)/tests/TestBuffer.xml \
	$(srcdir)/tests/TestBufferExternal.xml \
	$(srcdir)/tests/TestBufferExternal2.xml \
	$(srcdir)/tests/TestBufferMitredJoin.xml \
	$(srcdir)/tests/heisenbugs.xml \
	$(srcdir)/tests/badguy3.xml \
	$(srcdir)/tests/hole_from_shell.xml \
	$(srcdir)/tests/hole_red.xml

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

XMLTester_SOURCES = XMLTester.cpp markup/MarkupSTL.cpp markup/MarkupSTL.h XMLTester.h BufferResultMatcher.h BufferResultMatcher.cpp
XMLTester_LDADD = $(LIBS)

INCLUDES = -I$(top_srcdir)/source/headers
INCLUDES += -I$(top_srcdir)/source/io/markup

static:
	$(CXX) $(CXXFLAGS) -I../headers -o staticXMLTester XMLTester.cpp MarkupSTL.o ../../source/geom/.libs/libgeos.a

test: XMLTester
	./XMLTester $(XMLTESTS) 

testrunner: testrunner.sh Makefile
	sh $(srcdir)/testrunner.sh $@ $(SAFE_XMLTESTS)

verbose-test: XMLTester
	./XMLTester -v $(XMLTESTS) 
