#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

check_PROGRAMS = TestSweepLineSpeed

LIBS = $(top_builddir)/src/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

TestSweepLineSpeed_SOURCES = TestSweepLineSpeed.cpp GeometryTestFactory.cpp bigtest.h
TestSweepLineSpeed_LDADD = $(LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = CMakeLists.txt

