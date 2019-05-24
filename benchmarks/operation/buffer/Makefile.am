#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = IteratedBufferStressTest

LIBS = $(top_builddir)/src/libgeos.la

IteratedBufferStressTest_SOURCES = IteratedBufferStressTest.cpp
IteratedBufferStressTest_LDADD = $(LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = CMakeLists.txt
