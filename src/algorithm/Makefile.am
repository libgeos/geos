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
    BoundaryNodeRule.cpp \
    Centroid.cpp \
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

# Deprecated files
# (http://geos.osgeo.org/pipermail/geos-devel/2006-March/001828.html):
#NonRobustLineIntersector.cpp
#RobustLineIntersector.cpp

libalgorithm_la_LIBADD =  \
        locate/liblocation.la \
        distance/libdistance.la
