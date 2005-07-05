prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = SimpleWKTTester
bin_PROGRAMS = XMLTester
# CTS

LIBS = $(top_builddir)/source/geom/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

# CTS_SOURCES = CTS.cpp
# CTS_LDADD = $(LIBS)

SimpleWKTTester_SOURCES = SimpleWKTTester.cpp
SimpleWKTTester_LDADD = $(LIBS)

XMLTester_SOURCES = XMLTester.cpp ../io/markup/MarkupSTL.cpp ../io/markup/MarkupSTL.h test.xml XMLTester.h
XMLTester_LDADD = $(LIBS)

INCLUDES = -I$(top_srcdir)/source/headers
INCLUDES += -I$(top_srcdir)/source/io/markup

static:
	$(CXX) $(CXXFLAGS) -I../headers -o staticXMLTester XMLTester.cpp MarkupSTL.o ../../source/geom/.libs/libgeos.a
