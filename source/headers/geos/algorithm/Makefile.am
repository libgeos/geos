SUBDIRS = \
	locate \
	distance

EXTRA_DIST = 

# Notes:
#
# PointLocator.h is needed by DistanceOp.h and OverlayOp.h for composition
#                   needed by RelateComputer.h needed by RelateOp.h
#
# LineIntersector.h needed by GeometryGraphOperation.h needed by OverlayOp.h
#                   needed by RelateComputer.h needed by RelateOp.h
# 

geosdir = $(includedir)/geos/algorithm

geos_HEADERS = 				\
	LineIntersector.h		\
	PointLocator.h	

noinst_HEADERS = 			\
	Angle.h				\
	CGAlgorithms.h			\
	CentralEndpointIntersector.h	\
	CentroidArea.h			\
	CentroidLine.h			\
	CentroidPoint.h			\
	ConvexHull.h			\
	ConvexHull.inl			\
	HCoordinate.h			\
	InteriorPointArea.h		\
	InteriorPointLine.h		\
	InteriorPointPoint.h		\
	MCPointInRing.h			\
	MinimumDiameter.h		\
	NotRepresentableException.h	\
	PointInRing.h			\
	RayCrossingCounter.h		\
	RobustDeterminant.h		\
	SIRtreePointInRing.h		\
	SimplePointInRing.h	

