#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
SUBDIRS=\
    util \
    prep

noinst_LTLIBRARIES = \
    libgeom.la

AM_CPPFLAGS = -I$(top_srcdir)/include

libgeom_la_SOURCES = \
    Coordinate.cpp \
    CoordinateSequence.cpp \
    CoordinateArraySequence.cpp \
    CoordinateArraySequenceFactory.cpp \
    DefaultCoordinateSequenceFactory.cpp \
    Dimension.cpp \
    Envelope.cpp \
    Geometry.cpp \
    GeometryCollection.cpp \
    GeometryComponentFilter.cpp \
    GeometryFactory.cpp \
    HeuristicOverlay.cpp \
    IntersectionMatrix.cpp \
    LinearRing.cpp \
    LineSegment.cpp \
    LineString.cpp \
    Location.cpp \
    MultiLineString.cpp \
    MultiPoint.cpp \
    MultiPolygon.cpp \
    Point.cpp \
    Polygon.cpp \
    PrecisionModel.cpp \
    Position.cpp \
    Quadrant.cpp \
    Triangle.cpp


libgeom_la_LIBADD = \
    util/libgeomutil.la \
    prep/libgeomprep.la
