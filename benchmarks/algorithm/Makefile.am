#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = \
	InteriorPointAreaPerfTest \
	VoronoiPerfTest \
	UnaryUnionSegmentsPerfTest

InteriorPointAreaPerfTest_SOURCES = InteriorPointAreaPerfTest.cpp
InteriorPointAreaPerfTest_LDADD = $(top_builddir)/src/libgeos.la

VoronoiPerfTest_SOURCES = VoronoiPerfTest.cpp
VoronoiPerfTest_LDADD = $(top_builddir)/src/libgeos.la

UnaryUnionSegmentsPerfTest_SOURCES = UnaryUnionSegmentsPerfTest.cpp
UnaryUnionSegmentsPerfTest_LDADD = $(top_builddir)/src/libgeos.la

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_srcdir)/src/io/markup

EXTRA_DIST = CMakeLists.txt
