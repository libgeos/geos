# $Id: Makefile.am 2842 2010-01-13 02:01:12Z mloskot $
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
SUBDIRS = \
    locate \
    distance

noinst_LTLIBRARIES = libalgorithm.la

INCLUDES = -I$(top_srcdir)/include 

libalgorithm_la_SOURCES = \
    Angle.cpp \
    BoundaryNodeRule.cpp \
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
# (http://geos.refractions.net/pipermail/geos-devel/2006-March/001828.html):
#NonRobustLineIntersector.cpp
#RobustLineIntersector.cpp

libalgorithm_la_LIBADD =  \
        locate/liblocation.la \
        distance/libdistance.la
