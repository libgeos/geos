#prefix=@prefix@
#top_srcdir=@top_srcdir@
#top_builddir=@top_builddir@

XMLTESTS=testLeaksBig.xml hexwkb.xml test.xml linemerge.xml buffer.xml TestInteriorPoint.xml TestCentroid.xml robustness.xml

noinst_PROGRAMS = SimpleWKTTester
bin_PROGRAMS = XMLTester
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
