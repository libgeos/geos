#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = \
	perf_voronoi \
	perf_unaryunion_segments \
	perf_interiorpoint_area

LIBS = $(top_builddir)/src/libgeos.la

perf_voronoi_SOURCES = VoronoiPerfTest.cpp
perf_voronoi_LDADD = $(LIBS)

perf_unaryunion_segments_SOURCES = UnaryUnionSegmentsPerfTest.cpp
perf_unaryunion_LDADD = $(LIBS)

perf_interiorpoint_area_SOURCES = InteriorPointAreaPerfTest.cpp
perf_interiorpoint_area_LDADD = $(LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_srcdir)/src/io/markup

EXTRA_DIST = CMakeLists.txt

