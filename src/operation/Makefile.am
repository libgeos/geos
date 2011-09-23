# $Id: Makefile.am 3123 2010-11-29 09:33:05Z strk $
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
SUBDIRS = \
    buffer \
    distance \
    linemerge \
    overlay \
    polygonize \
    predicate \
    relate \
    union \
    sharedpaths \
    valid       

noinst_LTLIBRARIES = liboperation.la

INCLUDES = -I$(top_srcdir)/include 

liboperation_la_SOURCES = \
	GeometryGraphOperation.cpp \
	IsSimpleOp.cpp 

liboperation_la_LIBADD = \
    buffer/libopbuffer.la \
    distance/libopdistance.la \
    linemerge/liboplinemerge.la \
    overlay/libopoverlay.la \
    polygonize/liboppolygonize.la \
    predicate/liboppredicate.la \
    relate/liboprelate.la \
    union/libopunion.la \
    valid/libopvalid.la \
    sharedpaths/libopsharedpaths.la
