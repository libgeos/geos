prefix=@prefix@
top_srcdir=@top_srcdir@
GEOS_VERSION=@VERSION@

pkglibdir=$(prefix)/lib
pkglib_LTLIBRARIES = libgeos.la

INCLUDES += -I$(top_srcdir)/source/headers -DGEOS_VERSION=\""$(GEOS_VERSION)"\"

DIR_SOURCES =		Coordinate.cpp GeometryFactory.cpp MultiPoint.cpp \
			CoordinateList.cpp IntersectionMatrix.cpp  \
			MultiPolygon.cpp Dimension.cpp LinearRing.cpp \
			Point.cpp Envelope.cpp LineSegment.cpp Polygon.cpp \
			GeometryCollection.cpp LineString.cpp \
			PrecisionModel.cpp GeometryCollectionIterator.cpp \
			Location.cpp Geometry.cpp MultiLineString.cpp

EXTERNAL_SOURCES = 	../graph/Depth.cpp  ../graph/GeometryGraph.cpp \
			../graph/DirectedEdge.cpp ../graph/GraphComponent.cpp \
			../graph/DirectedEdgeStar.cpp ../graph/Label.cpp \
			../graph/Edge.cpp ../graph/Node.cpp \
			../graph/EdgeEnd.cpp ../graph/NodeFactory.cpp \
			../graph/EdgeEndStar.cpp ../graph/NodeMap.cpp \
			../graph/EdgeIntersection.cpp ../graph/PlanarGraph.cpp\
			../graph/EdgeIntersectionList.cpp \
			../graph/Position.cpp ../graph/EdgeList.cpp \
			../graph/Quadrant.cpp \
			../graph/EdgeRing.cpp ../graph/TopologyLocation.cpp \
			../graph/index/MonotoneChain.cpp \
			../graph/index/MonotoneChainEdge.cpp \
			../graph/index/MonotoneChainIndexer.cpp \
			../graph/index/SegmentIntersector.cpp \
			../graph/index/SimpleMCSweepLineIntersector.cpp \
			../graph/index/SweepLineEvent.cpp \
			../graph/index/SweepLineSegment.cpp \
			../io/ParseException.cpp   ../io/WKTReader.cpp  \
			../io/Writer.cpp ../io/StringTokenizer.cpp  \
			../io/WKTWriter.cpp ../io/markup/MarkupSTL.cpp \
			../util/Assert.cpp  \
			../util/AssertionFailedException.cpp \
                        ../algorithm/CGAlgorithms.cpp \
			../algorithm/HCoordinate.cpp \
			../algorithm/LineIntersector.cpp \
			../algorithm/NotRepresentableException.cpp \
                        ../algorithm/NonRobustCGAlgorithms.cpp \
                        ../algorithm/NotRepresentableException.cpp \
			../algorithm/PointInRing.cpp \
			../algorithm/PointLocator.cpp \
			../algorithm/RobustCGAlgorithms.cpp \
			../algorithm/RobustDeterminant.cpp \
			../algorithm/RobustLineIntersector.cpp \
			../algorithm/SimplePointInAreaLocator.cpp \
			../algorithm/SimplePointInRing.cpp \
			../operation/relate/EdgeEndBuilder.cpp \
			../operation/relate/EdgeEndBundle.cpp \
			../operation/relate/EdgeEndBundleStar.cpp \
			../operation/relate/RelateComputer.cpp \
			../operation/relate/RelateNode.cpp \
			../operation/relate/RelateNodeFactory.cpp \
			../operation/relate/RelateNodeGraph.cpp \
			../operation/relate/RelateOp.cpp \
                        ../operation/GeometryGraphOperation.cpp \
			../operation/IsSimpleOp.cpp

libgeos_la_SOURCES =	$(DIR_SOURCES) $(EXTERNAL_SOURCES)

libgeos_la_LDFLAGS = -version-info 1

dist-local:
		cp -p $(DIR_SOURCES) Makefile.am Makefile.in $(distdir)
