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
                    GeometryComponentFilter.cpp \
                    GeometryFactory.cpp \
                    IntersectionMatrix.cpp  \
                    LinearRing.cpp \
                    LineSegment.cpp \
                    LineString.cpp \
                    Location.cpp \
                    MultiLineString.cpp \
                    MultiPoint.cpp \
                    MultiPolygon.cpp \
                    PointCoordinateList.cpp \
                    Point.cpp \
                    Polygon.cpp \
		    PrecisionModel.cpp \
                    TopologyException.cpp 

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
                    ../graph/index/SimpleEdgeSetIntersector.cpp \
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
                    ../util/GEOSException.cpp \
                    ../util/IllegalArgumentException.cpp \
                    ../util/UnsupportedOperationException.cpp \
                    ../bigtest/GeometryTestFactory.cpp \
                    ../algorithm/CGAlgorithms.cpp \
                    ../algorithm/HCoordinate.cpp \
                    ../algorithm/InteriorPointArea.cpp \
                    ../algorithm/LineIntersector.cpp \
                    ../algorithm/MCPointInRing.cpp \
                    ../algorithm/CentroidArea.cpp \
                    ../algorithm/NonRobustCGAlgorithms.cpp \
                    ../algorithm/NonRobustLineIntersector.cpp \
                    ../algorithm/NotRepresentableException.cpp \
                    ../algorithm/PointLocator.cpp \
                    ../algorithm/RobustCGAlgorithms.cpp \
                    ../algorithm/RobustDeterminant.cpp \
                    ../algorithm/RobustLineIntersector.cpp \
                    ../algorithm/SimplePointInAreaLocator.cpp \
                    ../algorithm/SimplePointInRing.cpp \
                    ../algorithm/CentroidLine.cpp \
                    ../algorithm/CentroidPoint.cpp \
                    ../algorithm/InteriorPointLine.cpp \
                    ../algorithm/InteriorPointPoint.cpp \
                    ../algorithm/SIRtreePointInRing.cpp \
                    ../operation/GeometryGraphOperation.cpp \
                    ../operation/IsSimpleOp.cpp \
                    ../operation/distance/ConnectedElementPointFilter.cpp \
                    ../operation/distance/DistanceOp.cpp \
                    ../operation/distance/LineExtracterFilter.cpp \
                    ../operation/distance/PointExtracterFilter.cpp \
                    ../operation/distance/PolygonExtracterFilter.cpp \
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
                    ../index/bintree/Bintree.cpp \
                    ../index/bintree/BinTreeInterval.cpp \
                    ../index/bintree/BinTreeNode.cpp \
                    ../index/bintree/Interval.cpp \
                    ../index/bintree/Key.cpp \
                    ../index/bintree/NodeBase.cpp \
                    ../index/bintree/Node.cpp \
                    ../index/bintree/Root.cpp \
                    ../index/chain/indexMonotoneChain.cpp \
                    ../index/chain/MonotoneChainBuilder.cpp \
                    ../index/chain/MonotoneChainOverlapAction.cpp \
                    ../index/chain/MonotoneChainSelectAction.cpp \
                    ../index/quadtree/DoubleBits.cpp \
                    ../index/quadtree/IntervalSize.cpp \
                    ../index/quadtree/Quadtree.cpp \
                    ../index/quadtree/QuadTreeKey.cpp \
                    ../index/quadtree/QuadTreeNodeBase.cpp \
                    ../index/quadtree/QuadTreeNode.cpp \
                    ../index/quadtree/QuadTreeRoot.cpp \
                    ../index/strtree/AbstractNode.cpp \
                    ../index/strtree/AbstractSTRtree.cpp \
                    ../index/strtree/Interval.cpp \
                    ../index/strtree/ItemBoundable.cpp \
                    ../index/strtree/SIRtree.cpp \
                    ../index/strtree/STRtree.cpp \
                    ../index/sweepline/indexSweepLineEvent.cpp \
                    ../index/sweepline/SweepLineIndex.cpp \
                    ../index/sweepline/SweepLineInterval.cpp 

libgeos_la_SOURCES = $(DIR_SOURCES) $(EXTERNAL_SOURCES)

libgeos_la_LDFLAGS = -version-info 1

dist-local:
	cp -p $(DIR_SOURCES) Makefile.am Makefile.in $(distdir)
