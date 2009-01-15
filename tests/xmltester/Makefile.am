#prefix=@prefix@
#top_srcdir=@top_srcdir@
#top_builddir=@top_builddir@

TESTS = testrunner

CLEANFILES = testrunner

EXTRA_DIST = testrunner.sh

SAFE_XMLTESTS=$(srcdir)/testLeaksBig.xml $(srcdir)/hexwkb.xml $(srcdir)/test.xml $(srcdir)/linemerge.xml $(srcdir)/TestInteriorPoint.xml $(srcdir)/TestCentroid.xml $(srcdir)/TestIsValid.xml $(srcdir)/TestValid.xml $(srcdir)/TestValid2.xml $(srcdir)/TestValid2-big.xml $(srcdir)/robustness.xml $(srcdir)/stmlf-cases-20061020.xml $(srcdir)/TestRobustOverlayFixed.xml $(srcdir)/fme.xml $(srcdir)/TestBufferExternal.xml $(srcdir)/heisenbugs.xml $(srcdir)/stmlf-cases-20070119.xml $(srcdir)/badguy3.xml $(srcdir)/hole_from_shell.xml $(srcdir)/hole_red.xml

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

XMLTester_SOURCES = XMLTester.cpp markup/MarkupSTL.cpp markup/MarkupSTL.h test.xml XMLTester.h
XMLTester_LDADD = $(LIBS)

INCLUDES = -I$(top_srcdir)/source/headers
INCLUDES += -I$(top_srcdir)/source/io/markup

static:
	$(CXX) $(CXXFLAGS) -I../headers -o staticXMLTester XMLTester.cpp MarkupSTL.o ../../source/geom/.libs/libgeos.a

test: XMLTester
	./XMLTester $(XMLTESTS) 

testrunner: testrunner.sh
	sh $(srcdir)/testrunner.sh $@ $(SAFE_XMLTESTS)

verbose-test: XMLTester
	./XMLTester -v $(XMLTESTS) 
