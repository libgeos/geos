SUBDIRS = index

noinst_LTLIBRARIES = libgeomgraph.la

INCLUDES = -I$(top_srcdir)/source/headers 

libgeomgraph_la_SOURCES = \
	Depth.cpp \
	DirectedEdge.cpp \
	DirectedEdgeStar.cpp \
	Edge.cpp \
	EdgeEnd.cpp \
	EdgeEndStar.cpp \
	EdgeIntersection.cpp \
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
