# $Id: Makefile.am 2842 2010-01-13 02:01:12Z mloskot $
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
SUBDIRS = snapround

noinst_LTLIBRARIES = libnoding.la

INCLUDES = -I$(top_srcdir)/include 

libnoding_la_SOURCES = \
	BasicSegmentString.cpp \
	FastNodingValidator.cpp \
	FastSegmentSetIntersectionFinder.cpp \
	IntersectionAdder.cpp \
	IntersectionFinderAdder.cpp \
	IteratedNoder.cpp \
	MCIndexNoder.cpp \
	MCIndexSegmentSetMutualIntersector.cpp \
	NodedSegmentString.cpp \
	NodingValidator.cpp \
	Octant.cpp \
	OrientedCoordinateArray.cpp \
	ScaledNoder.cpp \
	SegmentIntersectionDetector.cpp \
	SegmentNode.cpp \
	SegmentNodeList.cpp \
	SegmentString.cpp \
	SegmentStringUtil.cpp \
	SimpleNoder.cpp \
	SingleInteriorIntersectionFinder.cpp

libnoding_la_LIBADD = snapround/libsnapround.la
