#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = \
	GeosOp

GeosOp_SOURCES = \
	GeosOp.cpp \
	GeosOp.h \
	cxxopts.hpp \
	GeomFunction.cpp \
	GeomFunction.h \
	WKBStreamReader.cpp \
	WKBStreamReader.h \
	WKTStreamReader.cpp \
	WKTStreamReader.h

GeosOp_LDADD = $(top_builddir)/src/libgeos.la

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = CMakeLists.txt
