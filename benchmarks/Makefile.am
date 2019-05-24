#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
SUBDIRS = \
	algorithm \
	operation \
	capi

LIBS = $(top_builddir)/src/libgeos.la

AM_CPPFLAGS = -I$(top_srcdir)/include

noinst_PROGRAMS = ClassSizes

ClassSizes_SOURCES = ClassSizes.cpp

EXTRA_DIST = CMakeLists.txt
