SUBDIRS = index

#EXTRA_DIST = 

# NOTES:
#
# EdgeList.h is needed by OverlayOp.h for composition
#
# PlanarGraph.h is needed by OverlayOp.h for inlines
#
# Depth.h needed by Edge.h
#
# EdgeRing.h needed by operation/overlay/{Minimal,Maximal}EdgeRing.h
#	for inheritance, but they are NOT installed
#
# Node.h needed by NodeMap.h for invariant testing
#
# NodeMap.h needed by RelateComputer.h needed by RelateOp.h
#	also needed by PlanarGraph.h for typedefs
#
# NodeFactory.h needed by operation/overlay/OverlayNodeFactory.h
#      and operation/relate/RelateNodeFactory.h for inheritance.
#      Both NOT installed
#
# GraphComponent.h needed by Node.h and Edge.h for inheritance
#
# EdgeEnd.h needed by Node.h for invariant tester
#
# EdgeEndStar.h needed by Node.h for invariant tester
#
# Depth.h needed by Edge.h for composition, which is NOT installed
#
#

geosdir = $(includedir)/geos/geomgraph

geos_HEADERS = 			\
	EdgeEnd.h		\
	EdgeEndStar.h		\
	EdgeList.h		\
	GeometryGraph.h		\
	GeometryGraph.inl	\
	GraphComponent.h	\
	Node.h			\
	NodeMap.h		\
	PlanarGraph.h

noinst_HEADERS = 		\
	Depth.h			\
	DirectedEdge.h		\
	DirectedEdge.inl	\
	DirectedEdgeStar.h	\
	Edge.h			\
	EdgeIntersection.h	\
	EdgeIntersectionList.h	\
	EdgeNodingValidator.h	\
	EdgeRing.h		\
	Label.h			\
	NodeFactory.h		\
	Position.h		\
	Quadrant.h		\
	TopologyLocation.h
