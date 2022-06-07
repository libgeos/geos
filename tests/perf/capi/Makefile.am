#
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = \
	perf_geospreparedcontains \
	memleak_mp_prep \
	perf_intersection

LIBS = $(top_builddir)/capi/libgeos_c.la
AM_CPPFLAGS = -I$(top_builddir)/capi -I$(top_srcdir)/include

memleak_mp_prep_SOURCES = memleak_mp_prep.c
memleak_mp_prep_LDADD = $(LIBS)

perf_intersection_SOURCES = IntersectionPerfTest.cpp
perf_intersection_LDADD = $(LIBS)

perf_geospreparedcontains_SOURCES = GEOSPreparedContainsPerfTest.cpp
perf_geospreparedcontains_LDADD = $(LIBS)

EXTRA_DIST = CMakeLists.txt


