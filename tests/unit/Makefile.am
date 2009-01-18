
INCLUDES = -I$(top_srcdir)/source/headers -I$(top_srcdir)/tests/tut -I$(top_builddir)/capi

TESTS = geos_unit

EXTRA_DIST = \
	capi/brokengrammar \
	capi/test.expected \
	capi/test.wkt \
	capi/testrunner.sh


#noinst_PROGRAMS = geos_unit
check_PROGRAMS = geos_unit geostest threadtest badthreadtest
#bin_PROGRAMS = geos_tut

geos_unit_LDADD = \
	$(top_builddir)/source/libgeos.la  \
	$(top_builddir)/capi/libgeos_c.la  \
	$(NULL)


geos_unit_SOURCES = geos_unit.cpp \
	algorithm/CGAlgorithms/isCCWTest.cpp \
	algorithm/CGAlgorithms/isPointInRingTest.cpp \
	algorithm/CGAlgorithms/computeOrientationTest.cpp \
	algorithm/ConvexHullTest.cpp \
	algorithm/PointLocatorTest.cpp \
	geom/CoordinateArraySequenceFactoryTest.cpp \
	geom/CoordinateArraySequenceTest.cpp \
	geom/CoordinateListTest.cpp \
	geom/CoordinateTest.cpp \
	geom/DimensionTest.cpp \
	geom/EnvelopeTest.cpp \
	geom/Geometry/isRectangleTest.cpp \
	geom/Geometry/coversTest.cpp \
	geom/GeometryFactoryTest.cpp \
	geom/IntersectionMatrixTest.cpp \
	geom/LinearRingTest.cpp \
	geom/LineSegmentTest.cpp \
	geom/LineStringTest.cpp \
	geom/LocationTest.cpp \
	geom/MultiLineStringTest.cpp \
	geom/MultiPointTest.cpp \
	geom/MultiPolygonTest.cpp \
	geom/PointTest.cpp \
	geom/PolygonTest.cpp \
	geom/prep/PreparedGeometryFactoryTest.cpp \
	geom/TriangleTest.cpp \
	index/quadtree/DoubleBitsTest.cpp \
	io/ByteOrderValuesTest.cpp \
	io/WKBReaderTest.cpp \
	noding/SegmentNodeTest.cpp \
	noding/SegmentPointComparatorTest.cpp \
	noding/SegmentStringTest.cpp \
	operation/distance/DistanceOpTest.cpp \
	operation/IsSimpleOpTest.cpp \
	operation/overlay/FuzzyPointLocatorTest.cpp \
	operation/overlay/OffsetPointGeneratorTest.cpp \
	operation/overlay/OverlayResultValidatorTest.cpp \
	operation/union/CascadedPolygonUnionTest.cpp \
	precision/GeometrySnapperTest.cpp \
	precision/LineStringSnapperTest.cpp \
	precision/SimpleGeometryPrecisionReducerTest.cpp \
	simplify/DouglasPeuckerSimplifierTest.cpp \
	simplify/TopologyPreservingSimplifierTest.cpp \
	util/UniqueCoordinateArrayFilterTest.cpp \
	capi/GEOSCoordSeqTest.cpp \
	capi/GEOSGeomToWKTTest.cpp \
	capi/GEOSSimplifyTest.cpp \
	capi/GEOSPolygonizer_getCutEdgesTest.cpp

noinst_HEADERS = \
	utility.h

geostest_SOURCES = capi/geostest.c
geostest_LDADD = $(top_builddir)/capi/libgeos_c.la

threadtest_SOURCES = capi/threadtest.c
threadtest_LDADD = $(top_builddir)/capi/libgeos_c.la -lpthread

badthreadtest_SOURCES = capi/badthreadtest.c
badthreadtest_LDADD = $(top_builddir)/capi/libgeos_c.la -lpthread

