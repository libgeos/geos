
INCLUDES = -I$(top_srcdir)/source/headers -I$(top_srcdir)/tests/tut

TESTS = geos_unit

#noinst_PROGRAMS = geos_unit
check_PROGRAMS = geos_unit
#bin_PROGRAMS = geos_tut

geos_unit_LDADD = $(top_builddir)/source/libgeos.la 

geos_unit_SOURCES = geos_unit.cpp \
	algorithm/PointLocatorTest.cpp \
	geom/CoordinateArraySequenceFactoryTest.cpp \
	geom/CoordinateArraySequenceTest.cpp \
	geom/CoordinateTest.cpp \
	geom/DimensionTest.cpp \
	geom/EnvelopeTest.cpp \
	geom/GeometryFactoryTest.cpp \
	geom/IntersectionMatrixTest.cpp \
	geom/LineSegmentTest.cpp \
	geom/LineStringTest.cpp \
	geom/LinearRingTest.cpp \
	geom/LocationTest.cpp \
	geom/MultiLineStringTest.cpp \
	geom/MultiPointTest.cpp \
	geom/MultiPolygonTest.cpp \
	geom/PointTest.cpp \
	geom/PolygonTest.cpp \
	geom/TriangleTest.cpp \
	geom/Geometry/isRectangleTest.cpp \
	index/quadtree/DoubleBitsTest.cpp \
	io/WKBReaderTest.cpp \
	noding/SegmentStringTest.cpp \
	simplify/DouglasPeuckerSimplifierTest.cpp \
	util/UniqueCoordinateArrayFilterTest.cpp

