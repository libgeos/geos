prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = TestSweepLineSpeed

LIBS = $(top_builddir)/source/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

TestSweepLineSpeed_SOURCES = TestSweepLineSpeed.cpp GeometryTestFactory.cpp bigtest.h
TestSweepLineSpeed_LDADD = $(LIBS)

INCLUDES = -I$(top_srcdir)/source/headers
INCLUDES += -I$(top_srcdir)/source/io/markup

#EXTRA_DIST = GeometryTestFactory.cpp

