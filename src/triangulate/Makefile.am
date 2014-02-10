#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
SUBDIRS = \
    quadedge

noinst_LTLIBRARIES = libtriangulate.la

AM_CPPFLAGS = -I$(top_srcdir)/include 

libtriangulate_la_SOURCES = \
	IncrementalDelaunayTriangulator.cpp \
	DelaunayTriangulationBuilder.cpp \
	VoronoiDiagramBuilder.cpp

libtriangulate_la_LIBADD = \
    quadedge/libquadedge.la
