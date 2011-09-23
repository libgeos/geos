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

INCLUDES = -I$(top_srcdir)/include
INCLUDES += -I$(top_srcdir)/src/io/markup
