# $Id: Makefile.am 3333 2011-05-06 18:51:56Z strk $
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
SUBDIRS = index

noinst_LTLIBRARIES = libgeomgraph.la

INCLUDES = -I$(top_srcdir)/include 

libgeomgraph_la_SOURCES = \
	Depth.cpp \
	DirectedEdge.cpp \
	DirectedEdgeStar.cpp \
	Edge.cpp \
	EdgeEnd.cpp \
	EdgeEndStar.cpp \
	EdgeIntersectionList.cpp \
	EdgeNodingValidator.cpp \
	EdgeList.cpp \
	EdgeRing.cpp \
	GeometryGraph.cpp \
	GraphComponent.cpp \
	Label.cpp \
	Node.cpp \
	NodeFactory.cpp \
	NodeMap.cpp \
	PlanarGraph.cpp \
	Position.cpp \
	Quadrant.cpp \
	TopologyLocation.cpp 

libgeomgraph_la_LIBADD = index/libgeomgraphindex.la
