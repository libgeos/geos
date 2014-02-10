#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = RectangleIntersectsPerfTest

LIBS = $(top_builddir)/src/libgeos.la

RectangleIntersectsPerfTest_SOURCES = RectangleIntersectsPerfTest.cpp 
RectangleIntersectsPerfTest_LDADD = $(LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_srcdir)/src/io/markup
