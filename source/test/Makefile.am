noinst_PROGRAMS = CTS SimpleWKTTester XMLTester

LIBS = ../geom/libgeos.la
#LIBS += ../graph/libgraph.a
#LIBS += ../graph/index/libindex.a
#LIBS += ../io/libio.a
#LIBS += ../io/markup/libmarkup.a
#LIBS += ../util/libutil.a

CTS_SOURCES = CTS.cpp
CTS_LDADD = $(LIBS)

SimpleWKTTester_SOURCES = SimpleWKTTester.cpp
SimpleWKTTester_LDADD = $(LIBS)

XMLTester_SOURCES = XMLTester.cpp
XMLTester_LDADD = $(LIBS)

INCLUDES = -I../headers
INCLUDES += -I../io/markup







