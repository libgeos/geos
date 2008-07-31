
INCLUDES = -I$(top_srcdir)/source/headers -I$(top_srcdir)/tests/tut -I$(top_builddir)/capi

TESTS = geos_unit

#noinst_PROGRAMS = geos_unit
check_PROGRAMS = geos_unit
#bin_PROGRAMS = geos_tut

geos_unit_LDADD = \
	$(top_builddir)/source/libgeos.la  \
	$(top_builddir)/capi/libgeos_c.la  \
	$(NULL)


geos_unit_SOURCES = geos_unit.cpp \
	algorithm/ConvexHullTest.cpp \
	algorithm/PointLocatorTest.cpp \
	algorithm/CGAlgorithms/isCCWTest.cpp \
	algorithm/CGAlgorithms/isPointInRingTest.cpp \
	algorithm/CGAlgorithms/computeOrientationTest.cpp \
	geom/CoordinateArraySequenceFactoryTest.cpp \
	geom/CoordinateArraySequenceTest.cpp \
	geom/CoordinateListTest.cpp \
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
	geom/Geometry/coversTest.cpp \
	index/quadtree/DoubleBitsTest.cpp \
	io/WKBReaderTest.cpp \
	io/ByteOrderValuesTest.cpp \
	operation/IsSimpleOpTest.cpp \
	operation/distance/DistanceOpTest.cpp \
	operation/overlay/FuzzyPointLocatorTest.cpp \
	operation/overlay/OffsetPointGeneratorTest.cpp \
	operation/overlay/OverlayResultValidatorTest.cpp \
	noding/SegmentNodeTest.cpp \
	noding/SegmentPointComparatorTest.cpp \
	noding/SegmentStringTest.cpp \
	precision/GeometrySnapperTest.cpp \
	precision/LineStringSnapperTest.cpp \
	precision/SimpleGeometryPrecisionReducerTest.cpp \
	simplify/DouglasPeuckerSimplifierTest.cpp \
	simplify/TopologyPreservingSimplifierTest.cpp \
	util/UniqueCoordinateArrayFilterTest.cpp \
	capi/GEOSCoordSeqTest.cpp	\
	capi/GEOSSimplifyTest.cpp \
	capi/GEOSPolygonizer_getCutEdgesTest.cpp

noinst_HEADERS = \
	utility.h
