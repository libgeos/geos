#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
SUBDIRS = \
    buffer \
    distance \
    intersection \
    linemerge \
    overlay \
    overlayng \
    polygonize \
    predicate \
    relate \
    union \
    sharedpaths \
    valid       

noinst_LTLIBRARIES = liboperation.la

AM_CPPFLAGS = -I$(top_srcdir)/include 

liboperation_la_SOURCES = \
	GeometryGraphOperation.cpp \
	BoundaryOp.cpp \
	IsSimpleOp.cpp 

liboperation_la_LIBADD = \
    buffer/libopbuffer.la \
    distance/libopdistance.la \
    intersection/libopintersection.la \
    linemerge/liboplinemerge.la \
    overlay/libopoverlay.la \
    overlayng/libopoverlayng.la \
    polygonize/liboppolygonize.la \
    predicate/liboppredicate.la \
    relate/liboprelate.la \
    union/libopunion.la \
    valid/libopvalid.la \
    sharedpaths/libopsharedpaths.la
