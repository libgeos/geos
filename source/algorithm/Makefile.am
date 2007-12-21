SUBDIRS = \
	locate

noinst_LTLIBRARIES = libalgorithm.la

INCLUDES = -I$(top_srcdir)/source/headers 

libalgorithm_la_SOURCES = \
	CentroidArea.cpp \
	CentroidLine.cpp \
	CentroidPoint.cpp \
	CGAlgorithms.cpp \
	ConvexHull.cpp \
	HCoordinate.cpp \
	InteriorPointArea.cpp \
	InteriorPointLine.cpp \
	InteriorPointPoint.cpp \
	LineIntersector.cpp \
	MCPointInRing.cpp \
	MinimumDiameter.cpp \
	NotRepresentableException.cpp \
	PointLocator.cpp \
	RayCrossingCounter.cpp \
	RobustDeterminant.cpp \
	SimplePointInRing.cpp \
	SIRtreePointInRing.cpp 

libalgorithm_la_LIBADD =  \
        location/liblocation.la           

