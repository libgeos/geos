prefix=@prefix@
top_srcdir=@top_srcdir@
GEOS_VERSION=@VERSION@

pkglibdir=$(prefix)/lib
pkglib_LTLIBRARIES = libgeos.la

INCLUDES = -I$(top_srcdir)/source/headers -DGEOS_VERSION=\""$(GEOS_VERSION)"\"

DIR_SOURCES =       BasicCoordinateList.cpp \
                    Coordinate.cpp \
                    CoordinateList.cpp \
                    CoordinateListFactory.cpp \
                    Dimension.cpp \
                    Envelope.cpp \
                    Geometry.cpp \
                    GeometryCollection.cpp \
                    GeometryCollectionIterator.cpp \
                    GeometryFactory.cpp \
                    IntersectionMatrix.cpp  \
                    LinearRing.cpp \
                    LineSegment.cpp \
                    LineString.cpp \
                    Location.cpp \
                    MultiLineString.cpp \
                    MultiPoint.cpp \
                    MultiPolygon.cpp \
                    Point.cpp \
                    PointCoordinateList.cpp \
                    Polygon.cpp \
                    PrecisionModel.cpp 

EXTERNAL_SOURCES =  ../graph/Depth.cpp \
                    ../graph/DirectedEdge.cpp \
                    ../graph/DirectedEdgeStar.cpp \
                    ../graph/Edge.cpp \
                    ../graph/EdgeEnd.cpp \
                    ../graph/EdgeEndStar.cpp \
                    ../graph/EdgeIntersection.cpp \
                    ../graph/EdgeIntersectionList.cpp \
                    ../graph/EdgeList.cpp \
                    ../graph/EdgeRing.cpp \
                    ../graph/GeometryGraph.cpp \
                    ../graph/GraphComponent.cpp \
                    ../graph/Label.cpp \
                    ../graph/Node.cpp \
                    ../graph/NodeFactory.cpp \
                    ../graph/NodeMap.cpp \
                    ../graph/PlanarGraph.cpp\
                    ../graph/Position.cpp \
                    ../graph/Quadrant.cpp \
                    ../graph/TopologyLocation.cpp \
                    ../graph/index/MonotoneChain.cpp \
                    ../graph/index/MonotoneChainEdge.cpp \
                    ../graph/index/MonotoneChainIndexer.cpp \
                    ../graph/index/SegmentIntersector.cpp \
                    ../graph/index/SimpleMCSweepLineIntersector.cpp \
                    ../graph/index/SweepLineEvent.cpp \
                    ../graph/index/SweepLineSegment.cpp \
                    ../io/ParseException.cpp \
                    ../io/StringTokenizer.cpp  \
                    ../io/WKTReader.cpp \
                    ../io/WKTWriter.cpp \
                    ../io/Writer.cpp \
                    ../io/markup/MarkupSTL.cpp \
                    ../util/Assert.cpp  \
                    ../util/AssertionFailedException.cpp \
                    ../bigtest/GeometryTestFactory.cpp \
                    ../algorithm/CGAlgorithms.cpp \
                    ../algorithm/HCoordinate.cpp \
                    ../algorithm/IntTreePointInRing.cpp \
                    ../algorithm/LineIntersector.cpp \
                    ../algorithm/MCPointInRing.cpp \
                    ../algorithm/NonRobustCGAlgorithms.cpp \
                    ../algorithm/NonRobustLineIntersector.cpp \
                    ../algorithm/NotRepresentableException.cpp \
                    ../algorithm/PointLocator.cpp \
                    ../algorithm/RobustCGAlgorithms.cpp \
                    ../algorithm/RobustDeterminant.cpp \
                    ../algorithm/RobustLineIntersector.cpp \
                    ../algorithm/SimplePointInAreaLocator.cpp \
                    ../algorithm/SimplePointInRing.cpp \
                    ../operation/GeometryGraphOperation.cpp \
                    ../operation/IsSimpleOp.cpp \
                    ../operation/overlay/EdgeSetNoder.cpp \
                    ../operation/overlay/LineBuilder.cpp \
                    ../operation/overlay/MaximalEdgeRing.cpp \
                    ../operation/overlay/MinimalEdgeRing.cpp \
                    ../operation/overlay/OverlayNodeFactory.cpp \
                    ../operation/overlay/OverlayOp.cpp \
                    ../operation/overlay/PointBuilder.cpp \
                    ../operation/overlay/PolygonBuilder.cpp \
                    ../operation/relate/EdgeEndBuilder.cpp \
                    ../operation/relate/EdgeEndBundle.cpp \
                    ../operation/relate/EdgeEndBundleStar.cpp \
                    ../operation/relate/RelateComputer.cpp \
                    ../operation/relate/RelateNode.cpp \
                    ../operation/relate/RelateNodeFactory.cpp \
                    ../operation/relate/RelateNodeGraph.cpp \
                    ../operation/relate/RelateOp.cpp \
                    ../operation/valid/ConnectedInteriorTester.cpp \
                    ../operation/valid/ConsistentAreaTester.cpp \
                    ../operation/valid/IsValidOp.cpp \
                    ../operation/valid/QuadtreeNestedRingTester.cpp \
                    ../operation/valid/RepeatedPointTester.cpp \
                    ../operation/valid/SimpleNestedRingTester.cpp \
                    ../operation/valid/SweeplineNestedRingTester.cpp \
                    ../operation/valid/TopologyValidationError.cpp \
                    ../index/chain/indexMonotoneChain.cpp \
                    ../index/chain/MonotoneChainBuilder.cpp \
                    ../index/chain/MonotoneChainOverlapAction.cpp \
                    ../index/chain/MonotoneChainSelectAction.cpp \
                    ../index/intervaltree/IntervalNode.cpp \
                    ../index/intervaltree/IntervalTree.cpp \
                    ../index/quadtree/Quad.cpp \
                    ../index/quadtree/Quadtree.cpp \
                    ../index/sweepline/indexSweepLineEvent.cpp \
                    ../index/sweepline/SweepLineIndex.cpp \
                    ../index/sweepline/SweepLineInterval.cpp 

libgeos_la_SOURCES = $(DIR_SOURCES) $(EXTERNAL_SOURCES)

libgeos_la_LDFLAGS = -version-info 1

dist-local:
                cp -p $(DIR_SOURCES) Makefile.am Makefile.in $(distdir)
