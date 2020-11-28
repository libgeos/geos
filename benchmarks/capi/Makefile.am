#
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

check_PROGRAMS = \
	memleak_mp_prep \
	GEOSPreparedContainsPerfTest \
	IntersectionPerfTest \
	UnaryOpPerfTest

LIBS = $(top_builddir)/capi/libgeos_c.la
AM_CPPFLAGS = -I$(top_builddir)/capi -I$(top_srcdir)/include

memleak_mp_prep_SOURCES = memleak_mp_prep.c
memleak_mp_prep_LDADD = $(LIBS)

GEOSPreparedContainsPerfTest_SOURCES = GEOSPreparedContainsPerfTest.cpp
GEOSPreparedContainsPerfTest_LDADD = $(top_builddir)/src/libgeos.la

IntersectionPerfTest_SOURCES = IntersectionPerfTest.cpp
IntersectionPerfTest_LDADD = $(top_builddir)/src/libgeos.la

UnaryOpPerfTest_SOURCES = UnaryOpPerfTest.cpp
UnaryOpPerfTest_LDADD = $(top_builddir)/src/libgeos.la

EXTRA_DIST = CMakeLists.txt
