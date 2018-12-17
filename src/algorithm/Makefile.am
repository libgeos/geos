#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
SUBDIRS = \
	locate \
	distance

noinst_LTLIBRARIES = libalgorithm.la

AM_CPPFLAGS = -I$(top_srcdir)/include

libalgorithm_la_SOURCES = \
	Angle.cpp \
	Area.cpp \
	BoundaryNodeRule.cpp \
	Centroid.cpp \
	CGAlgorithms.cpp \
	CGAlgorithmsDD.cpp \
	ConvexHull.cpp \
	HCoordinate.cpp \
	InteriorPointArea.cpp \
	InteriorPointLine.cpp \
	InteriorPointPoint.cpp \
	LineIntersector.cpp \
	MinimumDiameter.cpp \
	NotRepresentableException.cpp \
	PointLocator.cpp \
	RayCrossingCounter.cpp \
	RayCrossingCounterDD.cpp \
	RobustDeterminant.cpp


libalgorithm_la_LIBADD = \
	locate/liblocation.la \
	distance/libdistance.la
