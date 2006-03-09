/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_OPBUFFER_H
#define GEOS_OPBUFFER_H

#include <geos/platform.h>
#include <geos/operation.h>
#include <geos/opOverlay.h>
#include <geos/geomgraph.h>
#include <geos/noding.h>
#include <geos/geom.h>
#include <geos/util/TopologyException.h>
#include <set>
#include <vector>

// Forward declarations
namespace geos {
	namespace algorithm {
		class CGAlgorithms;
	}
}

namespace geos {
namespace operation { // geos.operation

/// Provides classes for computing buffers of geometries
namespace buffer { // geos.operation.buffer

class BufferBuilder;
class BufferOp;
class BufferSubgraph;
class DepthSegment;
class OffsetCurveBuilder;
class OffsetCurveSetBuilder;
class RightmostEdgeFinder;
class SubgraphDepthLocater;

/*
 * \class RightmostEdgeFinder opBuffer.h geos/opBuffer.h
 *
 * \brief
 * A RightmostEdgeFinder find the geomgraph::DirectedEdge in a list which has
 * the highest coordinate, and which is oriented L to R at that point.
 * (I.e. the right side is on the RHS of the edge.)
 */
class RightmostEdgeFinder {
private:
	//algorithm::CGAlgorithms* cga;
	int minIndex;
	geom::Coordinate minCoord;
	geomgraph::DirectedEdge *minDe;
	geomgraph::DirectedEdge *orientedDe;
	void findRightmostEdgeAtNode();
	void findRightmostEdgeAtVertex();
	void checkForRightmostCoordinate(geomgraph::DirectedEdge *de);
	int getRightmostSide(geomgraph::DirectedEdge *de, int index);
	int getRightmostSideOfSegment(geomgraph::DirectedEdge *de, int i);

public:

	/*
	 * A RightmostEdgeFinder finds the geomgraph::DirectedEdge with the
	 * rightmost coordinate.
	 * The geomgraph::DirectedEdge returned is guranteed to have the R of
	 * the world on its RHS.
	 */
	RightmostEdgeFinder();
	inline geomgraph::DirectedEdge* getEdge();
	inline geom::Coordinate& getCoordinate();
	void findEdge(std::vector<geomgraph::DirectedEdge*>* dirEdgeList);
};

// INLINES
geomgraph::DirectedEdge* RightmostEdgeFinder::getEdge() { return orientedDe; }
geom::Coordinate& RightmostEdgeFinder::getCoordinate() { return minCoord; }

/*
 * \class BufferSubgraph opBuffer.h geos/opBuffer.h
 *
 * \brief A connected subset of the graph of DirectedEdges and geomgraph::Node.
 * 
 * Its edges will generate either
 * - a single polygon in the complete buffer, with zero or more holes, or
 * -  ne or more connected holes
 */
class BufferSubgraph {
private:
	RightmostEdgeFinder finder;

	std::vector<geomgraph::DirectedEdge*> dirEdgeList;

	std::vector<geomgraph::Node*> nodes;

	geom::Coordinate *rightMostCoord;

	geom::Envelope *env;

	/*
	 * Adds all nodes and edges reachable from this node to the subgraph.
	 * Uses an explicit stack to avoid a large depth of recursion.
	 *
	 * @param node a node known to be in the subgraph
	 */
	void addReachable(geomgraph::Node *startNode);

	/*
	 * Adds the argument node and all its out edges to the subgraph
	 * @param node the node to add
	 * @param nodeStack the current set of nodes being traversed
	 */
	void add(geomgraph::Node *node,std::vector<geomgraph::Node*> *nodeStack);

	void clearVisitedEdges();

	/*
	 * Compute depths for all dirEdges via breadth-first traversal
	 * of nodes in graph
	 * @param startEdge edge to start processing with
	 */
	// <FIX> MD - use iteration & queue rather than recursion, for speed and robustness
	void computeDepths(geomgraph::DirectedEdge *startEdge);

	void computeNodeDepth(geomgraph::Node *n);
	void copySymDepths(geomgraph::DirectedEdge *de);
	bool contains(std::set<geomgraph::Node*>&nodes,geomgraph::Node *node);

public:

	/*
	 * algorithm::CGAlgorithms arg kept for backward-compatibility.
	 * It is unused.
	 */
	BufferSubgraph(algorithm::CGAlgorithms *cga=NULL);
	~BufferSubgraph();

	inline std::vector<geomgraph::DirectedEdge*>* getDirectedEdges();

	inline std::vector<geomgraph::Node*>* getNodes();

	/*
	 * Gets the rightmost coordinate in the edges of the subgraph
	 */
	inline geom::Coordinate* getRightmostCoordinate();

	/*
	 * Creates the subgraph consisting of all edges reachable from
	 * this node.
	 * Finds the edges in the graph and the rightmost coordinate.
	 *
	 * @param node a node to start the graph traversal from
	 */
	void create(geomgraph::Node *node);

	void computeDepth(int outsideDepth);

	/*
	 * Find all edges whose depths indicates that they are in the
	 * result area(s).
	 * Since we want polygon shells to be
	 * oriented CW, choose dirEdges with the interior of the result
	 * on the RHS.
	 * Mark them as being in the result.
	 * Interior Area edges are the result of dimensional collapses.
	 * They do not form part of the result area boundary.
	 */
	void findResultEdges();

	/*
	 * BufferSubgraphs are compared on the x-value of their rightmost
	 * Coordinate.
	 * This defines a partial ordering on the graphs such that:
	 * 
	 * g1 >= g2 <==> Ring(g2) does not contain Ring(g1)
	 *
	 * where Polygon(g) is the buffer polygon that is built from g.
	 *
	 * This relationship is used to sort the BufferSubgraphs so
	 * that shells are guaranteed to
	 * be built before holes.
	 */
	int compareTo(BufferSubgraph *);

	/**
	 * Computes the envelope of the edges in the subgraph.
	 * The envelope is cached after being computed.
	 *
	 * @return the envelope of the graph.
	 */
	geom::Envelope *getEnvelope();
};

// INLINES
geom::Coordinate* BufferSubgraph::getRightmostCoordinate() {return rightMostCoord;}
std::vector<geomgraph::Node*>* BufferSubgraph::getNodes() { return &nodes; }
std::vector<geomgraph::DirectedEdge*>* BufferSubgraph::getDirectedEdges() {
	return &dirEdgeList;
}

/**
 * \class OffsetCurveBuilder opBuffer.h geos/opBuffer.h
 *
 * \brief
 * Computes the raw offset curve for a
 * single Geometry component (ring, line or point).
 *
 * A raw offset curve line is not noded -
 * it may contain self-intersections (and usually will).
 * The final buffer polygon is computed by forming a topological graph
 * of all the noded raw curves and tracing outside contours.
 * The points in the raw curve are rounded to the required precision model.
 *
 * Last port: operation/buffer/OffsetCurveBuilder.java rev. 1.7 
 *
 */
class OffsetCurveBuilder {
public:
	/** \brief
	 * The default number of facets into which to divide a fillet
	 * of 90 degrees.
	 *
	 * A value of 8 gives less than 2% max error in the buffer distance.
	 * For a max error of < 1%, use QS = 12
	 */
	static const int DEFAULT_QUADRANT_SEGMENTS=8;

	OffsetCurveBuilder(const geom::PrecisionModel *newPrecisionModel,
			int quadrantSegments=DEFAULT_QUADRANT_SEGMENTS);

	~OffsetCurveBuilder();

	void setEndCapStyle(int newEndCapStyle);

	/**
	 * This method handles single points as well as lines.
	 * Lines are assumed to <b>not</b> be closed (the function will not
	 * fail for closed lines, but will generate superfluous line caps).
	 *
	 * @param lineList the std::vector to which CoordinateSequences will
	 *                 be pushed_back
	 */
	void getLineCurve(const geom::CoordinateSequence* inputPts, double distance,
		std::vector<geom::CoordinateSequence*>& lineList);

	/**
	 * This method handles the degenerate cases of single points and lines,
	 * as well as rings.
	 *
	 * @param lineList the std::vector to which CoordinateSequences will
	 *                 be pushed_back
	 */
	void getRingCurve(const geom::CoordinateSequence *inputPts, int side,
		double distance, std::vector<geom::CoordinateSequence*>& lineList);


private:

	static const double PI; //  3.14159265358979

	static const double MAX_CLOSING_SEG_LEN; // 3.0

	algorithm::LineIntersector li;

	/** \brief
	 * The angle quantum with which to approximate a fillet curve
	 * (based on the input # of quadrant segments)
	 */
	double filletAngleQuantum;

	/** \brief
	 * the max error of approximation between a quad segment and
	 * the true fillet curve
	 */
	double maxCurveSegmentError;

	/// Owned by this object, destroyed by dtor 
	//
	/// This actually gets created multiple times
	/// and each of the old versions is pushed
	/// to the ptLists std::vector to ensure all
	/// created CoordinateSequences are properly 
	/// destroyed.
	///
	geom::CoordinateSequence *ptList;

	double distance;

	const geom::PrecisionModel *precisionModel;

	int endCapStyle;

	int joinStyle;

	geom::Coordinate s0, s1, s2;

	geom::LineSegment seg0;

	geom::LineSegment seg1;

	geom::LineSegment offset0;

	geom::LineSegment offset1;

	int side;

//	static geom::CoordinateSequence* copyCoordinates(geom::CoordinateSequence *pts);

	void init(double newDistance);

	geom::CoordinateSequence* getCoordinates();

	void computeLineBufferCurve(const geom::CoordinateSequence& inputPts);

	void computeRingBufferCurve(const geom::CoordinateSequence& inputPts, int side);

	void addPt(const geom::Coordinate &pt);

	void closePts();

	void initSideSegments(const geom::Coordinate &nS1, const geom::Coordinate &nS2, int nSide);

	void addNextSegment(const geom::Coordinate &p, bool addStartPoint);

	/// Add last offset point
	void addLastSegment();

	/**
	 * Compute an offset segment for an input segment on a given side and at a
	 * given distance.
	 * The offset points are computed in full double precision, for accuracy.
	 *
	 * @param seg the segment to offset
	 * @param side the side of the segment the offset lies on
	 * @param distance the offset distance
	 * @param offset the points computed for the offset segment
	 */
	void computeOffsetSegment(const geom::LineSegment& seg, int side, double distance,
			geom::LineSegment& offset);

	/// Add an end cap around point p1, terminating a line segment coming from p0
	void addLineEndCap(const geom::Coordinate &p0,const geom::Coordinate &p1);

	/**
	 * @param p base point of curve
	 * @param p0 start point of fillet curve
	 * @param p1 endpoint of fillet curve
	 */
	void addFillet(const geom::Coordinate &p, const geom::Coordinate &p0,
			const geom::Coordinate &p1, int direction, double distance);

	/** \brief
	 * Adds points for a fillet. 
	 * The start and end point for the fillet are not added -
	 * the caller must add them if required.
	 *
	 * @param direction is -1 for a CW angle, 1 for a CCW angle
	 */
	void addFillet(const geom::Coordinate &p, double startAngle, double endAngle,
			int direction, double distance);

	/// Adds a CW circle around a point
	void addCircle(const geom::Coordinate &p, double distance);

	/// Adds a CW square around a point
	void addSquare(const geom::Coordinate &p, double distance);

	std::vector<geom::CoordinateSequence *>ptLists;
};

// INLINES
inline void OffsetCurveBuilder::setEndCapStyle(int newEndCapStyle) {
	endCapStyle=newEndCapStyle;
}


/**
 * \class BufferOp opBuffer.h geos/opBuffer.h
 *
 * \brief
 * Computes the buffer of a geometry, for both positive and negative
 * buffer distances.
 *
 * In GIS, the buffer of a geometry is defined as
 * the Minkowski sum or difference of the geometry
 * with a circle with radius equal to the absolute value of the buffer
 * distance.
 * In the CAD/CAM world buffers are known as </i>offset curves</i>.
 * In morphological analysis they are known as <i>erosion</i> and <i>dilation</i>.
 * 
 * Since true buffer curves may contain circular arcs,
 * computed buffer polygons can only be approximations to the true geometry.
 * The user can control the accuracy of the curve approximation by specifying
 * the number of linear segments with which to approximate a curve.
 * 
 * The end cap style of a linear buffer may be specified.
 * The following end cap styles are supported:
 * - CAP_ROUND - the usual round end caps
 * - CAP_BUTT - end caps are truncated flat at the line ends
 * - CAP_SQUARE - end caps are squared off at the buffer distance
 *   beyond the line ends
 * 
 * Last port: operation/buffer/BufferOp.java rev. 1.31 (JTS-1.7)
 *
 */
class BufferOp {


private:

	static int MAX_PRECISION_DIGITS;

	/**
	 * Compute a reasonable scale factor to limit the precision of
	 * a given combination of Geometry and buffer distance.
	 * The scale factor is based on a heuristic.
	 *
	 * @param g the Geometry being buffered
	 *
	 * @param distance the buffer distance
	 *
	 * @param maxPrecisionDigits the mzx # of digits that should be
	 *        allowed by the precision determined by the
	 *        computed scale factor
	 *
	 * @return a scale factor that allows a reasonable amount of
	 *         precision for the buffer computation
	 */
	static double precisionScaleFactor(const geom::Geometry *g,
			double distance, int maxPrecisionDigits);

	const geom::Geometry *argGeom;

	util::TopologyException saveException;

	double distance;

	int quadrantSegments;

	int endCapStyle;

	geom::Geometry* resultGeometry;

	void computeGeometry();

	void bufferOriginalPrecision();

	void bufferReducedPrecision(int precisionDigits);

	void bufferReducedPrecision();

	void bufferFixedPrecision(const geom::PrecisionModel& fixedPM);

public:

	enum {
		/// Specifies a round line buffer end cap style.
		CAP_ROUND,
		/// Specifies a butt (or flat) line buffer end cap style.
		CAP_BUTT,
		/// Specifies a square line buffer end cap style.
		CAP_SQUARE
	};

	/**
	 * Computes the buffer for a geometry for a given buffer distance
	 * and accuracy of approximation.
	 *
	 * @param g the geometry to buffer
	 * @param distance the buffer distance
	 * @param quadrantSegments the number of segments used to
	 *        approximate a quarter circle
	 * @return the buffer of the input geometry
	 *
	 */
	static geom::Geometry* bufferOp(const geom::Geometry *g,
		double distance,
		int quadrantSegments=
			OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS,
		int endCapStyle=BufferOp::CAP_ROUND);

	/**
	 * Initializes a buffer computation for the given geometry
	 *
	 * @param g the geometry to buffer
	 */
	BufferOp(const geom::Geometry *g)
		:
		argGeom(g),
		quadrantSegments(OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS),
		endCapStyle(BufferOp::CAP_ROUND),
		resultGeometry(NULL)
	{}

	/**
	 * Specifies the end cap style of the generated buffer.
	 * The styles supported are CAP_ROUND, CAP_BUTT, and CAP_SQUARE.
	 * The default is CAP_ROUND.
	 *
	 * @param endCapStyle the end cap style to specify
	 */
	inline void setEndCapStyle(int nEndCapStyle);

	/**
	 * Specifies the end cap style of the generated buffer.
	 * The styles supported are CAP_ROUND, CAP_BUTT, and CAP_SQUARE.
	 * The default is CAP_ROUND.
	 *
	 * @param endCapStyle the end cap style to specify
	 */
	inline void setQuadrantSegments(int nQuadrantSegments);

	/**
	 * Returns the buffer computed for a geometry for a given buffer
	 * distance.
	 *
	 * @param g the geometry to buffer
	 * @param distance the buffer distance
	 * @return the buffer of the input geometry
	 */
	geom::Geometry* getResultGeometry(double nDistance);

	/**
	 * Comutes the buffer for a geometry for a given buffer distance
	 * and accuracy of approximation.
	 *
	 * @param g the geometry to buffer
	 * @param distance the buffer distance
	 * @param quadrantSegments the number of segments used to
	 * approximate a quarter circle
	 * @return the buffer of the input geometry
	 *
	 * @deprecated use setQuadrantSegments instead
	 */
	geom::Geometry* getResultGeometry(double nDistance, int nQuadrantSegments);
};

// BufferOp inlines
void BufferOp::setQuadrantSegments(int q) { quadrantSegments=q; }
void BufferOp::setEndCapStyle(int s) { endCapStyle=s; }


/**
 * \class OffsetCurveSetBuilder opBuffer.h geos/opBuffer.h
 *
 * \brief
 * Creates all the raw offset curves for a buffer of a Geometry.
 *
 * Raw curves need to be noded together and polygonized to form the
 * final buffer area.
 *
 * Last port: operation/buffer/OffsetCurveSetBuilder.java rev. 1.7 (JTS-1.7)
 *
 */
class OffsetCurveSetBuilder {

private:

	// To keep track of newly-created Labels.
	// Labels will be relesed by object dtor
	std::vector<geomgraph::Label*> newLabels;

	const geom::Geometry& inputGeom;

	double distance;

	OffsetCurveBuilder& curveBuilder;

	/// The raw offset curves computed.
	/// This class holds ownership of std::vector elements.
	///
	std::vector<noding::SegmentString*> curveList;

	/**
	 * Creates a noding::SegmentString for a coordinate list which is a raw
	 * offset curve, and adds it to the list of buffer curves.
	 * The noding::SegmentString is tagged with a geomgraph::Label giving the topology
	 * of the curve.
	 * The curve may be oriented in either direction.
	 * If the curve is oriented CW, the locations will be:
	 * - Left: Location.EXTERIOR
	 * - Right: Location.INTERIOR
	 */
	void addCurve(geom::CoordinateSequence *coord, int leftLoc,
			int rightLoc);

	void add(const geom::Geometry& g);

	void addCollection(const geom::GeometryCollection *gc);

	/**
	 * Add a Point to the graph.
	 */
	void addPoint(const geom::Point *p);

	void addLineString(const geom::LineString *line);

	void addPolygon(const geom::Polygon *p);

	/**
	 * Add an offset curve for a ring.
	 * The side and left and right topological location arguments
	 * assume that the ring is oriented CW.
	 * If the ring is in the opposite orientation,
	 * the left and right locations must be interchanged and the side
	 * flipped.
	 *
	 * @param coord the coordinates of the ring (must not contain
	 * repeated points)
	 * @param offsetDistance the distance at which to create the buffer
	 * @param side the side of the ring on which to construct the buffer
	 *             line
	 * @param cwLeftLoc the location on the L side of the ring
	 *                  (if it is CW)
	 * @param cwRightLoc the location on the R side of the ring
	 *                   (if it is CW)
	 */
	void addPolygonRing(const geom::CoordinateSequence *coord,
			double offsetDistance, int side, int cwLeftLoc,
			int cwRightLoc);

	/**
	 * The ringCoord is assumed to contain no repeated points.
	 * It may be degenerate (i.e. contain only 1, 2, or 3 points).
	 * In this case it has no area, and hence has a minimum diameter of 0.
	 *
	 * @param ringCoord
	 * @param offsetDistance
	 * @return
	 */
	bool isErodedCompletely(geom::CoordinateSequence *ringCoord,
			double bufferDistance);

	/**
	 * Tests whether a triangular ring would be eroded completely by
	 * the given buffer distance.
	 * This is a precise test.  It uses the fact that the inner buffer
	 * of a triangle converges on the inCentre of the triangle (the
	 * point equidistant from all sides).  If the buffer distance is
	 * greater than the distance of the inCentre from a side, the
	 * triangle will be eroded completely.
	 *
	 * This test is important, since it removes a problematic case where
	 * the buffer distance is slightly larger than the inCentre distance.
	 * In this case the triangle buffer curve "inverts" with incorrect
	 * topology, producing an incorrect hole in the buffer.
	 *
	 * @param triangleCoord
	 * @param bufferDistance
	 * @return
	 */
	bool isTriangleErodedCompletely(geom::CoordinateSequence *triangleCoord,
			double bufferDistance);

public:
	OffsetCurveSetBuilder(const geom::Geometry& newInputGeom,
		double newDistance, OffsetCurveBuilder& newCurveBuilder);

	~OffsetCurveSetBuilder();

	/**
	 * Computes the set of raw offset curves for the buffer.
	 * Each offset curve has an attached {@link geomgraph::Label} indicating
	 * its left and right location.
	 *
	 * @return a Collection of SegmentStrings representing the raw
	 * buffer curves
	 */
	std::vector<noding::SegmentString*>& getCurves();

	void addCurves(const std::vector<geom::CoordinateSequence*>& lineList,
		int leftLoc, int rightLoc);

};

/*
 * \class DepthSegment opBuffer.h geos/opBuffer.h
 *
 * \brief
 * A segment from a directed edge which has been assigned a depth value
 * for its sides.
 */
class DepthSegment {
private:
	geom::LineSegment upwardSeg;

	/**
	 * Compare two collinear segments for left-most ordering.
	 * If segs are vertical, use vertical ordering for comparison.
	 * If segs are equal, return 0.
	 * Segments are assumed to be directed so that the second
	 * coordinate is >= to the first
	 * (e.g. up and to the right).
	 *
	 * @param seg0 a segment to compare
	 * @param seg1 a segment to compare
	 * @return
	 */
	int compareX(geom::LineSegment *seg0, geom::LineSegment *seg1);

public:
	int leftDepth;
	DepthSegment(const geom::LineSegment &seg, int depth);
	~DepthSegment();

	/**
	 * Defines a comparision operation on DepthSegments
	 * which orders them left to right
	 *
	 * <pre>
	 * DS1 < DS2   if   DS1.seg is left of DS2.seg
	 * DS1 > DS2   if   DS1.seg is right of DS2.seg
	 * </pre>
	 *
	 * @param obj
	 * @return
	 */
	int compareTo(DepthSegment *);
};

bool DepthSegmentLT(DepthSegment *first, DepthSegment *second);

/**
 * \class SubgraphDepthLocater opBuffer.h geos/opBuffer.h
 *
 * \brief
 * Locates a subgraph inside a set of subgraphs,
 * in order to determine the outside depth of the subgraph.
 *
 * The input subgraphs are assumed to have had depths
 * already calculated for their edges.
 *
 * Last port: operation/buffer/SubgraphDepthLocater.java rev. 1.5 (JTS-1.7)
 */
class SubgraphDepthLocater {

public:

	SubgraphDepthLocater(std::vector<BufferSubgraph*> *newSubgraphs)
		:
		subgraphs(newSubgraphs)
	{}

	~SubgraphDepthLocater() {}

	int getDepth(geom::Coordinate &p);

private:

	std::vector<BufferSubgraph*> *subgraphs;

	geom::LineSegment seg;

	/**
	 * Finds all non-horizontal segments intersecting the stabbing line.
	 * The stabbing line is the ray to the right of stabbingRayLeftPt.
	 *
	 * @param stabbingRayLeftPt the left-hand origin of the stabbing line
	 * @param stabbedSegments a vector to which DepthSegments intersecting
	 *        the stabbing line will be added.
	 */
	void findStabbedSegments(geom::Coordinate &stabbingRayLeftPt,
			std::vector<DepthSegment*>& stabbedSegments);

	/**
	 * Finds all non-horizontal segments intersecting the stabbing line
	 * in the list of dirEdges.
	 * The stabbing line is the ray to the right of stabbingRayLeftPt.
	 *
	 * @param stabbingRayLeftPt the left-hand origin of the stabbing line
	 * @param stabbedSegments the current vector of DepthSegments
	 *        intersecting the stabbing line will be added.
	 */
	void findStabbedSegments(geom::Coordinate &stabbingRayLeftPt,
			std::vector<geomgraph::DirectedEdge*> *dirEdges,
			std::vector<DepthSegment*>& stabbedSegments);

	/**
	 * Finds all non-horizontal segments intersecting the stabbing line
	 * in the input dirEdge.
	 * The stabbing line is the ray to the right of stabbingRayLeftPt.
	 *
	 * @param stabbingRayLeftPt the left-hand origin of the stabbing line
	 * @param stabbedSegments the current list of DepthSegments intersecting
	 *        the stabbing line
	 */
	void findStabbedSegments(geom::Coordinate &stabbingRayLeftPt,
			geomgraph::DirectedEdge *dirEdge,
			std::vector<DepthSegment*>& stabbedSegments);

};

bool BufferSubgraphGT(BufferSubgraph *first, BufferSubgraph *second);

/**
 *
 * \brief
 * Builds the buffer geometry for a given input geometry and precision model.
 *
 * Allows setting the level of approximation for circular arcs,
 * and the precision model in which to carry out the computation.
 * 
 * When computing buffers in floating point double-precision
 * it can happen that the process of iterated noding can fail to converge
 * (terminate).
 *
 * In this case a TopologyException will be thrown.
 * Retrying the computation in a fixed precision
 * can produce more robust results.
 *
 * Last port: operation/buffer/BufferBuilder.java rev. 1.21 (JTS-1.7)
 *
 */
class BufferBuilder {
friend class Unload;

private:
	/**
	 * Compute the change in depth as an edge is crossed from R to L
	 */
	static int depthDelta(geomgraph::Label *label);

	int quadrantSegments;

	int endCapStyle;

	const geom::PrecisionModel* workingPrecisionModel;

	algorithm::LineIntersector* li;

	noding::IntersectionAdder* intersectionAdder;

	noding::Noder* workingNoder;

	const geom::GeometryFactory* geomFact;

	geomgraph::EdgeList edgeList;

	std::vector<geomgraph::Label *> newLabels;

	void computeNodedEdges(std::vector<noding::SegmentString*>& bufferSegStrList,
			const geom::PrecisionModel *precisionModel);
			// throw(GEOSException);

	/**
	 * Inserted edges are checked to see if an identical edge already
	 * exists.
	 * If so, the edge is not inserted, but its label is merged
	 * with the existing edge.
	 *
	 * The function takes responsability of releasing the Edge parameter
	 * memory when appropriate.
	 */
	void insertEdge(geomgraph::Edge *e);

	void createSubgraphs(geomgraph::PlanarGraph *graph,
			std::vector<BufferSubgraph*>& list);

	/**
	 * Completes the building of the input subgraphs by
	 * depth-labelling them,
	 * and adds them to the PolygonBuilder.
	 * The subgraph list must be sorted in rightmost-coordinate order.
	 *
	 * @param subgraphList the subgraphs to build
	 * @param polyBuilder the PolygonBuilder which will build
	 *        the final polygons
	 */
	void buildSubgraphs(std::vector<BufferSubgraph*> *subgraphList,
			overlay::PolygonBuilder *polyBuilder);

	/// \brief
	/// Return the externally-set noding::Noder OR a newly created
	/// one using the given precisionModel.
	//
	/// NOTE: if an externally-set noding::Noder is available no
	/// check is performed to ensure it will use the
	/// given PrecisionModel
	///
	noding::Noder* getNoder(const geom::PrecisionModel* precisionModel);


public:
	/**
	 * Creates a new BufferBuilder
	 */
	BufferBuilder()
		:
		quadrantSegments(OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS),
		endCapStyle(BufferOp::CAP_ROUND),
		workingPrecisionModel(NULL),
		li(NULL),
		intersectionAdder(NULL),
		workingNoder(NULL),
		geomFact(NULL),
		edgeList()
	{}

	~BufferBuilder();

	/**
	 * Sets the number of segments used to approximate a angle fillet
	 *
	 * @param quadrantSegments the number of segments in a fillet for
	 *  a quadrant
	 */
	void setQuadrantSegments(int nQuadrantSegments) {
		quadrantSegments=nQuadrantSegments;
	} 


	/**
	 * Sets the precision model to use during the curve computation
	 * and noding,
	 * if it is different to the precision model of the Geometry.
	 * If the precision model is less than the precision of the
	 * Geometry precision model,
	 * the Geometry must have previously been rounded to that precision.
	 *
	 * @param pm the precision model to use
	 */
	void setWorkingPrecisionModel(const geom::PrecisionModel *pm) {
		workingPrecisionModel=pm;
	}

	/**
	 * Sets the {@link noding::Noder} to use during noding.
	 * This allows choosing fast but non-robust noding, or slower
	 * but robust noding.
	 *
	 * @param noder the noder to use
	 */
	void setNoder(noding::Noder* newNoder) { workingNoder = newNoder; }

	void setEndCapStyle(int nEndCapStyle) {
		endCapStyle=nEndCapStyle;
	}

	geom::Geometry* buffer(const geom::Geometry *g, double distance);
		// throw (GEOSException);

};

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

#endif // ndef GEOS_OPBUFFER_H

/**********************************************************************
 * $Log$
 * Revision 1.26  2006/03/09 17:40:28  strk
 * Fixed bug#33 (hopefully)
 *
 * Revision 1.25  2006/03/09 16:46:48  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.24  2006/03/07 14:20:14  strk
 * Big deal of heap allocations reduction
 *
 * Revision 1.23  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.22  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.21  2006/03/01 17:16:39  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.20  2006/02/28 19:22:20  strk
 * Fixed in-place definition of static members in OffsetCurveBuilder (bug#33)
 *
 * Revision 1.19  2006/02/28 14:34:05  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
 * Revision 1.18  2006/02/23 20:05:19  strk
 * Fixed bug in MCIndexNoder constructor making memory checker go crazy, more
 * doxygen-friendly comments, miscellaneous cleanups
 *
 * Revision 1.17  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.16  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.15  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.14  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.13  2006/02/14 13:28:25  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.12  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.11  2006/02/08 17:18:28  strk
 * - New WKTWriter::toLineString and ::toPoint convenience methods
 * - New IsValidOp::setSelfTouchingRingFormingHoleValid method
 * - New Envelope::centre()
 * - New Envelope::intersection(Envelope)
 * - New Envelope::expandBy(distance, [ydistance])
 * - New LineString::reverse()
 * - New MultiLineString::reverse()
 * - New Geometry::buffer(distance, quadSeg, endCapStyle)
 * - Obsoleted toInternalGeometry/fromInternalGeometry
 * - More const-correctness in Buffer "package"
 *
 **********************************************************************/

