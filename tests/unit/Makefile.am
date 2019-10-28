#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#

AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/tests/unit/tut -I$(top_builddir)/capi

TESTS = geos_unit

EXTRA_DIST = \
	tut/tut_assert.hpp \
	tut/tut_config.hpp \
	tut/tut_console_reporter.hpp \
	tut/tut_cppunit_reporter.hpp \
	tut/tut_exception.hpp \
	tut/tut_fpt.hpp \
	tut/tut.hpp \
	tut/tut_macros.hpp \
	tut/tut_main.hpp \
	tut/tut_posix.hpp \
	tut/tut_reporter.hpp \
	tut/tut_restartable.hpp \
	tut/tut_result.hpp \
	tut/tut_runner.hpp \
	tut/tut_xml_reporter.hpp \
	CMakeLists.txt


#noinst_PROGRAMS = geos_unit
check_PROGRAMS = geos_unit
#bin_PROGRAMS = geos_tut

geos_unit_LDADD = \
	$(top_builddir)/src/libgeos.la  \
	$(top_builddir)/capi/libgeos_c.la  \
	$(NULL)


geos_unit_SOURCES = \
	geos_unit.cpp \
	algorithm/AngleTest.cpp \
	algorithm/AreaTest.cpp \
	algorithm/CGAlgorithms/computeOrientationTest.cpp \
	algorithm/CGAlgorithms/isCCWTest.cpp \
	algorithm/CGAlgorithms/isPointInRingTest.cpp \
	algorithm/CGAlgorithms/signedAreaTest.cpp \
	algorithm/ConvexHullTest.cpp \
	algorithm/distance/DiscreteFrechetDistanceTest.cpp \
	algorithm/distance/DiscreteHausdorffDistanceTest.cpp \
	algorithm/InteriorPointAreaTest.cpp \
	algorithm/IntersectionTest.cpp \
	algorithm/LengthTest.cpp \
	algorithm/LocatePointInRingTest.cpp \
	algorithm/MinimumBoundingCircleTest.cpp \
	algorithm/MinimumDiameterTest.cpp \
	algorithm/OrientationIndexFailureTest.cpp \
	algorithm/PointLocatorTest.cpp \
	algorithm/RobustLineIntersectionTest.cpp \
	algorithm/RobustLineIntersectorTest.cpp \
	capi/GEOSBufferTest.cpp \
	capi/GEOSBuildAreaTest.cpp \
	capi/GEOSCAPIDefinesTest.cpp \
	capi/GEOSClipByRectTest.cpp \
	capi/GEOSContainsTest.cpp \
	capi/GEOSConvexHullTest.cpp \
	capi/GEOSCoordSeqTest.cpp \
	capi/GEOSCoverageUnionTest.cpp \
	capi/GEOSDelaunayTriangulationTest.cpp \
	capi/GEOSDistanceTest.cpp \
	capi/GEOSEqualsTest.cpp \
	capi/GEOSFrechetDistanceTest.cpp \
	capi/GEOSGeom_createCollectionTest.cpp \
	capi/GEOSGeom_createTest.cpp \
	capi/GEOSGeom_extentTest.cpp \
	capi/GEOSGeom_extractUniquePointsTest.cpp \
	capi/GEOSGeomFromWKBTest.cpp \
	capi/GEOSGeom_setPrecisionTest.cpp \
	capi/GEOSGeomToWKTTest.cpp \
	capi/GEOSGetCentroidTest.cpp \
	capi/GEOSHausdorffDistanceTest.cpp \
	capi/GEOSInterpolateTest.cpp \
	capi/GEOSInterruptTest.cpp \
	capi/GEOSIntersectionTest.cpp \
	capi/GEOSIntersectsTest.cpp \
	capi/GEOSisClosedTest.cpp \
	capi/GEOSisValidDetailTest.cpp \
	capi/GEOSLineString_PointTest.cpp \
	capi/GEOSMakeValidTest.cpp \
	capi/GEOSMinimumBoundingCircleTest.cpp \
	capi/GEOSMinimumClearanceTest.cpp \
	capi/GEOSMinimumRectangleTest.cpp \
	capi/GEOSMinimumWidthTest.cpp \
	capi/GEOSNearestPointsTest.cpp \
	capi/GEOSNodeTest.cpp \
	capi/GEOSOffsetCurveTest.cpp \
	capi/GEOSOrientationIndexTest.cpp \
	capi/GEOSPointOnSurfaceTest.cpp \
	capi/GEOSPolygonizeTest.cpp \
	capi/GEOSPreparedGeometryTest.cpp \
	capi/GEOSRelateBoundaryNodeRuleTest.cpp \
	capi/GEOSRelatePatternMatchTest.cpp \
	capi/GEOSReverseTest.cpp \
	capi/GEOSSegmentIntersectionTest.cpp \
	capi/GEOSSharedPathsTest.cpp \
	capi/GEOSSimplifyTest.cpp \
	capi/GEOSSnapTest.cpp \
	capi/GEOSSTRtreeTest.cpp \
	capi/GEOSUnaryUnionTest.cpp \
	capi/GEOSUserDataTest.cpp \
	capi/GEOSVoronoiDiagramTest.cpp \
	capi/GEOSWithinTest.cpp \
	geom/CoordinateArraySequenceFactoryTest.cpp \
	geom/CoordinateArraySequenceTest.cpp \
	geom/CoordinateListTest.cpp \
	geom/CoordinateTest.cpp \
	geom/DimensionTest.cpp \
	geom/EnvelopeTest.cpp \
	geom/FixedSizeCoordinateSequenceTest.cpp \
	geom/Geometry/cloneTest.cpp \
	geom/GeometryCollectionTest.cpp \
	geom/GeometryComponentFilterTest.cpp \
	geom/Geometry/coversTest.cpp \
	geom/Geometry/equalsTest.cpp \
	geom/GeometryFactoryTest.cpp \
	geom/GeometryFilterTest.cpp \
	geom/Geometry/isRectangleTest.cpp \
	geom/Geometry/normalizeTest.cpp \
	geom/Geometry/touchesTest.cpp \
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
	geom/PrecisionModelTest.cpp \
	geom/prep/PreparedGeometryFactoryTest.cpp \
	geom/prep/PreparedGeometry/touchesTest.cpp \
	geom/TriangleTest.cpp \
	geom/util/GeometryExtracterTest.cpp \
	index/quadtree/DoubleBitsTest.cpp \
	index/strtree/SIRtreeTest.cpp \
	io/ByteOrderValuesTest.cpp \
	io/WKBReaderTest.cpp \
	io/WKBWriterTest.cpp \
	io/WKTReaderTest.cpp \
	io/WKTWriterTest.cpp \
	io/WriterTest.cpp \
	linearref/LengthIndexedLineTest.cpp \
	noding/BasicSegmentStringTest.cpp \
	noding/NodedSegmentStringTest.cpp \
	noding/OrientedCoordinateArrayTest.cpp \
	noding/SegmentNodeTest.cpp \
	noding/SegmentPointComparatorTest.cpp \
	noding/snapround/HotPixelTest.cpp \
	noding/snapround/MCIndexSnapRounderTest.cpp \
	operation/buffer/BufferBuilderTest.cpp \
	operation/buffer/BufferOpTest.cpp \
	operation/buffer/BufferParametersTest.cpp \
	operation/distance/DistanceOpTest.cpp \
	operation/distance/IndexedFacetDistanceTest.cpp \
	operation/geounion/CascadedPolygonUnionTest.cpp \
	operation/geounion/CoverageUnionTest.cpp \
	operation/geounion/UnaryUnionOpTest.cpp \
	operation/intersection/RectangleIntersectionTest.cpp \
	operation/IsSimpleOpTest.cpp \
	operation/linemerge/LineMergerTest.cpp \
	operation/linemerge/LineSequencerTest.cpp \
	operation/overlay/OverlayOpUnionTest.cpp \
	operation/overlay/snap/GeometrySnapperTest.cpp \
	operation/overlay/snap/LineStringSnapperTest.cpp \
	operation/overlay/validate/FuzzyPointLocatorTest.cpp \
	operation/overlay/validate/OffsetPointGeneratorTest.cpp \
	operation/overlay/validate/OverlayResultValidatorTest.cpp \
	operation/polygonize/PolygonizeTest.cpp \
	operation/sharedpaths/SharedPathsOpTest.cpp \
	operation/valid/IsValidOpTest.cpp \
	operation/valid/RepeatedPointRemoverTest.cpp \
	operation/valid/ValidClosedRingTest.cpp \
	operation/valid/ValidSelfTouchingRingFormingHoleTest.cpp \
	precision/CommonBitsTest.cpp \
	precision/GeometryPrecisionReducerTest.cpp \
	precision/SimpleGeometryPrecisionReducerTest.cpp \
	simplify/DouglasPeuckerSimplifierTest.cpp \
	simplify/TopologyPreservingSimplifierTest.cpp \
	triangulate/DelaunayTest.cpp \
	triangulate/quadedge/QuadEdgeSubdivisionTest.cpp \
	triangulate/quadedge/QuadEdgeTest.cpp \
	triangulate/quadedge/VertexTest.cpp \
	triangulate/VoronoiTest.cpp \
	util/UniqueCoordinateArrayFilterTest.cpp

noinst_HEADERS = \
	utility.h
