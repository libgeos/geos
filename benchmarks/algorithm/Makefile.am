#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = InteriorPointAreaPerfTest

LIBS = $(top_builddir)/src/libgeos.la

InteriorPointAreaPerfTest_SOURCES = InteriorPointAreaPerfTest.cpp
InteriorPointAreaPerfTest_LDADD = $(LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_srcdir)/src/io/markup

EXTRA_DIST = CMakeLists.txt
