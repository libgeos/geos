#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

check_PROGRAMS = bug234
check_PROGRAMS += TestSweepLineSpeed

LIBS = $(top_builddir)/src/libgeos.la
# -lmpatrol -lbfd -lintl -liberty -limagehlp

TestSweepLineSpeed_SOURCES = TestSweepLineSpeed.cpp GeometryTestFactory.cpp bigtest.h
TestSweepLineSpeed_LDADD = $(LIBS)

bug234_SOURCES = bug234.cpp
bug234_LDADD = $(LIBS)

INCLUDES = -I$(top_srcdir)/include
INCLUDES += -I$(top_srcdir)/src/io/markup

EXTRA_DIST = CMakeLists.txt

