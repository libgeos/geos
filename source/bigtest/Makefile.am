noinst_PROGRAMS = TestSweepLineSpeed

LIBS = ../geom/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

TestSweepLineSpeed_SOURCES = TestSweepLineSpeed.cpp GeometryTestFactory.cpp bigtest.h
TestSweepLineSpeed_LDADD = $(LIBS)

INCLUDES = -I../headers
INCLUDES += -I../io/markup

#EXTRA_DIST = GeometryTestFactory.cpp

