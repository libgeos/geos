noinst_PROGRAMS = SimpleWKTTester XMLTester
# CTS

LIBS = ../geom/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

# CTS_SOURCES = CTS.cpp
# CTS_LDADD = $(LIBS)

SimpleWKTTester_SOURCES = SimpleWKTTester.cpp
SimpleWKTTester_LDADD = $(LIBS)

XMLTester_SOURCES = XMLTester.cpp ../io/markup/MarkupSTL.cpp ../io/markup/MarkupSTL.h test.xml
XMLTester_LDADD = $(LIBS)

INCLUDES = -I../headers 
INCLUDES += -I../io/markup







