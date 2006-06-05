SUBDIRS = algorithm

#EXTRA_DIST = 

# Notes:
#
#  PlanarGraph.h is needed by LineMergeGraph.h (needed by LineMerger.h)
#                for inheritance
#
# NodeMap.h needed by PlanarGraph.h for composition
#
# Edge.h needed by linemerge/LineMergeEdge.h [not installed]
#	and polygonize/PolygonizeEdge.h [not installed]
#	for inheritance
#
# DirectedEdge.h needed by linemerge/LineMergeDirectedEdge.h [not installed]
#	and polygonize/PolygonizeDirectedEdge.h [not installed]
#	for inheritance
#
# DirectedEdgeStar.h needed by Node.h [not installed] for inlines
#
# GraphComponent.h needed by Node.h [not installed], Edge.h [not installed]
#	and DirectedEdge.h [not installed] for inheritance
#
#

geosdir = $(includedir)/geos/planargraph

geos_HEADERS =			\
	NodeMap.h		\
	PlanarGraph.h	

noinst_HEADERS = 		\
	DirectedEdge.h		\
	DirectedEdgeStar.h	\
	Edge.h			\
	GraphComponent.h	\
	Node.h			\
	Subgraph.h
