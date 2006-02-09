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

#include <memory>
#include <geos/platform.h>
#include <geos/operation.h>
#include <geos/opOverlay.h>
#include <geos/geomgraph.h>
#include <geos/noding.h>
#include <geos/geom.h>
#include <vector>

namespace geos {

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
 * A RightmostEdgeFinder find the DirectedEdge in a list which has
 * the highest coordinate, and which is oriented L to R at that point.
 * (I.e. the right side is on the RHS of the edge.)
 */
class RightmostEdgeFinder {
private:
	//CGAlgorithms* cga;
	int minIndex;
	Coordinate minCoord;
	DirectedEdge *minDe;
	DirectedEdge *orientedDe;
	void findRightmostEdgeAtNode();
	void findRightmostEdgeAtVertex();
	void checkForRightmostCoordinate(DirectedEdge *de);
	int getRightmostSide(DirectedEdge *de, int index);
	int getRightmostSideOfSegment(DirectedEdge *de, int i);

public:

	/*
	 * A RightmostEdgeFinder finds the DirectedEdge with the
	 * rightmost coordinate.
	 * The DirectedEdge returned is guranteed to have the R of
	 * the world on its RHS.
	 * The CGAlgorithms arg is kept for backward compatibility,
	 * there's no use for it.
	 */
	RightmostEdgeFinder(CGAlgorithms *newCga=NULL);
	inline DirectedEdge* getEdge();
	inline Coordinate& getCoordinate();
	void findEdge(vector<DirectedEdge*>* dirEdgeList);
};

// INLINES
DirectedEdge* RightmostEdgeFinder::getEdge() { return orientedDe; }
Coordinate& RightmostEdgeFinder::getCoordinate() { return minCoord; }

/*
 * \class BufferSubgraph opBuffer.h geos/opBuffer.h
 *
 * \brief A connected subset of the graph of DirectedEdges and Node.
 * 
 * Its edges will generate either
 * - a single polygon in the complete buffer, with zero or more holes, or
 * -  ne or more connected holes
 */
class BufferSubgraph {
private:
	RightmostEdgeFinder finder;

	vector<DirectedEdge*> dirEdgeList;

	vector<Node*> nodes;

	Coordinate *rightMostCoord;

	Envelope *env;

	/*
	 * Adds all nodes and edges reachable from this node to the subgraph.
	 * Uses an explicit stack to avoid a large depth of recursion.
	 *
	 * @param node a node known to be in the subgraph
	 */
	void addReachable(Node *startNode);

	/*
	 * Adds the argument node and all its out edges to the subgraph
	 * @param node the node to add
	 * @param nodeStack the current set of nodes being traversed
	 */
	void add(Node *node,vector<Node*> *nodeStack);

	void clearVisitedEdges();

	/*
	 * Compute depths for all dirEdges via breadth-first traversal
	 * of nodes in graph
	 * @param startEdge edge to start processing with
	 */
	// <FIX> MD - use iteration & queue rather than recursion, for speed and robustness
	void computeDepths(DirectedEdge *startEdge);

	void computeNodeDepth(Node *n);
	void copySymDepths(DirectedEdge *de);
	bool contains(set<Node*>&nodes,Node *node);

public:

	/*
	 * CGAlgorithms arg kept for backward-compatibility.
	 * It is unused.
	 */
	BufferSubgraph(CGAlgorithms *cga=NULL);
	~BufferSubgraph();

	inline vector<DirectedEdge*>* getDirectedEdges();

	inline vector<Node*>* getNodes();

	/*
	 * Gets the rightmost coordinate in the edges of the subgraph
	 */
	inline Coordinate* getRightmostCoordinate();

	/*
	 * Creates the subgraph consisting of all edges reachable from
	 * this node.
	 * Finds the edges in the graph and the rightmost coordinate.
	 *
	 * @param node a node to start the graph traversal from
	 */
	void create(Node *node);

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
	Envelope *getEnvelope();
};

// INLINES
Coordinate* BufferSubgraph::getRightmostCoordinate() {return rightMostCoord;}
vector<Node*>* BufferSubgraph::getNodes() { return &nodes; }
vector<DirectedEdge*>* BufferSubgraph::getDirectedEdges() {
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

	OffsetCurveBuilder(const PrecisionModel *newPrecisionModel);
	~OffsetCurveBuilder();
	OffsetCurveBuilder(const PrecisionModel *newPrecisionModel,int quadrantSegments);
	inline void setEndCapStyle(int newEndCapStyle);

	/**
	 * This method handles single points as well as lines.
	 * Lines are assumed to <b>not</b> be closed (the function will not
	 * fail for closed lines, but will generate superfluous line caps).
	 *
	 * @return a List of Coordinate[]
	 */
	vector<CoordinateSequence*>* getLineCurve(const CoordinateSequence *inputPts, double distance);

	/**
	 * This method handles the degenerate cases of single points and lines,
	 * as well as rings.
	 *
	 * @return a List of Coordinate[]
	 */
	vector<CoordinateSequence*>* getRingCurve(const CoordinateSequence *inputPts, int side, double distance);

private:

	static double PI_OVER_2;
	static double MAX_CLOSING_SEG_LEN;
//	static final Coordinate[] arrayTypeCoordinate = new Coordinate[0];
	//CGAlgorithms *cga;
	LineIntersector *li;

	/**
	 * The angle quantum with which to approximate a fillet curve
	 * (based on the input # of quadrant segments)
	 */
	double filletAngleQuantum;

	/**
	 * the max error of approximation between a quad segment and
	 * the true fillet curve
	 */
	double maxCurveSegmentError;

	CoordinateSequence *ptList;

	double distance;
	const PrecisionModel *precisionModel;
	int endCapStyle;
	int joinStyle;
	Coordinate s0, s1, s2;
	LineSegment *seg0;
	LineSegment *seg1;
	LineSegment *offset0;
	LineSegment *offset1;
	int side;
//	static CoordinateSequence* copyCoordinates(CoordinateSequence *pts);
	void init(double newDistance);
	CoordinateSequence* getCoordinates();
	void computeLineBufferCurve(const CoordinateSequence *inputPts);
	void computeRingBufferCurve(const CoordinateSequence *inputPts, int side);
	void addPt(const Coordinate &pt);
	void closePts();
	void initSideSegments(const Coordinate &nS1, const Coordinate &nS2, int nSide);
	void addNextSegment(const Coordinate &p, bool addStartPoint);
	/**
	* Add last offset point
	*/
	void addLastSegment();
	/**
	* Compute an offset segment for an input segment on a given side and at a given distance.
	* The offset points are computed in full double precision, for accuracy.
	*
	* @param seg the segment to offset
	* @param side the side of the segment the offset lies on
	* @param distance the offset distance
	* @param offset the points computed for the offset segment
	*/
	void computeOffsetSegment(LineSegment *seg, int side, double distance, LineSegment *offset);
	/**
	* Add an end cap around point p1, terminating a line segment coming from p0
	*/
	void addLineEndCap(const Coordinate &p0,const Coordinate &p1);
	/**
	* @param p base point of curve
	* @param p0 start point of fillet curve
	* @param p1 endpoint of fillet curve
	*/
	void addFillet(const Coordinate &p,const Coordinate &p0,const Coordinate &p1, int direction, double distance);
	/**
	* Adds points for a fillet.  The start and end point for the fillet are not added -
	* the caller must add them if required.
	*
	* @param direction is -1 for a CW angle, 1 for a CCW angle
	*/
	void addFillet(const Coordinate &p, double startAngle, double endAngle, int direction, double distance);
	/**
	* Adds a CW circle around a point
	*/
	void addCircle(const Coordinate &p, double distance);
	/**
	* Adds a CW square around a point
	*/
	void addSquare(const Coordinate &p, double distance);
private:
	vector<CoordinateSequence *>ptLists;
};

// INLINES
void OffsetCurveBuilder::setEndCapStyle(int newEndCapStyle) {
	endCapStyle=newEndCapStyle;
}


/*
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
 * In the CAD/CAM world buffers are known as </b>offset curves</b>.
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
 * The computation uses an algorithm involving iterated noding and
 * precision reduction to provide a high degree of robustness.
 */
class BufferOp {


private:

	static int MAX_PRECISION_DIGITS;

	/*
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
	static double precisionScaleFactor(const Geometry *g,
			double distance,int maxPrecisionDigits);

	const Geometry *argGeom;

	TopologyException saveException;

	double distance;

	int quadrantSegments;

	int endCapStyle;

	Geometry* resultGeometry;

	void computeGeometry();

	void bufferOriginalPrecision();

	void bufferFixedPrecision(int precisionDigits);

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
	 * Comutes the buffer for a geometry for a given buffer distance
	 * and accuracy of approximation.
	 *
	 * @param g the geometry to buffer
	 * @param distance the buffer distance
	 * @param quadrantSegments the number of segments used to
	 *        approximate a quarter circle
	 * @return the buffer of the input geometry
	 *
	 */
	static Geometry* bufferOp(const Geometry *g,
		double distance,
		int quadrantSegments=
			OffsetCurveBuilder::DEFAULT_QUADRANT_SEGMENTS,
		int endCapStyle=BufferOp::CAP_ROUND);

	/**
	 * Initializes a buffer computation for the given geometry
	 *
	 * @param g the geometry to buffer
	 */
	BufferOp(const Geometry *g);

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
	Geometry* getResultGeometry(double nDistance);

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
	Geometry* getResultGeometry(double nDistance, int nQuadrantSegments);
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
 */
class OffsetCurveSetBuilder {
public:
	OffsetCurveSetBuilder(const Geometry *newInputGeom,
		double newDistance, OffsetCurveBuilder *newCurveBuilder);

	~OffsetCurveSetBuilder();

	/**
	 * Computes the set of raw offset curves for the buffer.
	 * Each offset curve has an attached {@link Label} indicating
	 * its left and right location.
	 *
	 * @return a Collection of SegmentStrings representing the raw
	 * buffer curves
	 */
	vector<SegmentString*>* getCurves();

	void addCurves(const vector<CoordinateSequence*> *lineList,
		int leftLoc, int rightLoc);

private:
	vector<Label*> newLabels;
	//CGAlgorithms cga;

	const Geometry *inputGeom;
	double distance;
	OffsetCurveBuilder *curveBuilder;
	vector<SegmentString*> *curveList;
	/**
	* Creates a {@link SegmentString} for a coordinate list which is a raw offset curve,
	* and adds it to the list of buffer curves.
	* The SegmentString is tagged with a Label giving the topology of the curve.
	* The curve may be oriented in either direction.
	* If the curve is oriented CW, the locations will be:
	* <br>Left: Location.EXTERIOR
	* <br>Right: Location.INTERIOR
	*/
	void addCurve(const CoordinateSequence *coord, int leftLoc, int rightLoc);
	void add(const Geometry *g);
	void addCollection(const GeometryCollection *gc);
	/**
	* Add a Point to the graph.
	*/
	void addPoint(const Point *p);
	void addLineString(const LineString *line);
	void addPolygon(const Polygon *p);
	/**
	* Add an offset curve for a ring.
	* The side and left and right topological location arguments
	* assume that the ring is oriented CW.
	* If the ring is in the opposite orientation,
	* the left and right locations must be interchanged and the side flipped.
	*
	* @param coord the coordinates of the ring (must not contain repeated points)
	* @param offsetDistance the distance at which to create the buffer
	* @param side the side of the ring on which to construct the buffer line
	* @param cwLeftLoc the location on the L side of the ring (if it is CW)
	* @param cwRightLoc the location on the R side of the ring (if it is CW)
	*/
	void addPolygonRing(const CoordinateSequence *coord, double offsetDistance, int side, int cwLeftLoc, int cwRightLoc);
	/**
	 * The ringCoord is assumed to contain no repeated points.
	 * It may be degenerate (i.e. contain only 1, 2, or 3 points).
	 * In this case it has no area, and hence has a minimum diameter of 0.
	 *
	 * @param ringCoord
	 * @param offsetDistance
	 * @return
	 */
	bool isErodedCompletely(CoordinateSequence *ringCoord, double bufferDistance);
	//bool isErodedCompletely(CoordinateSequence *ringCoord, double bufferDistance) { return isErodedCompletely((const CoordinateSequence)ringCoord, bufferDistance) }

	/**
	* Tests whether a triangular ring would be eroded completely by the given
	* buffer distance.
	* This is a precise test.  It uses the fact that the inner buffer of a
	* triangle converges on the inCentre of the triangle (the point
	* equidistant from all sides).  If the buffer distance is greater than the
	* distance of the inCentre from a side, the triangle will be eroded completely.
	*
	* This test is important, since it removes a problematic case where
	* the buffer distance is slightly larger than the inCentre distance.
	* In this case the triangle buffer curve "inverts" with incorrect topology,
	* producing an incorrect hole in the buffer.
	*
	* @param triangleCoord
	* @param bufferDistance
	* @return
	*/
	bool isTriangleErodedCompletely(CoordinateSequence *triangleCoord, double bufferDistance);
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
	LineSegment upwardSeg;

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
	int compareX(LineSegment *seg0, LineSegment *seg1);

public:
	int leftDepth;
	DepthSegment(const LineSegment &seg, int depth);
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

/*
 * \class SubgraphDepthLocater opBuffer.h geos/opBuffer.h
 *
 * \brief Locates a subgraph inside a set of subgraphs,
 *
 * in order to determine the outside depth of the subgraph.
 * The input subgraphs are assumed to have had depths
 * already calculated for their edges.
 *
 */
class SubgraphDepthLocater {
public:
	SubgraphDepthLocater(vector<BufferSubgraph*> *newSubgraphs);
	~SubgraphDepthLocater();
	int getDepth(Coordinate &p);
private:
	vector<BufferSubgraph*> *subgraphs;
	LineSegment seg;
	//CGAlgorithms *cga;
	/**
	 * Finds all non-horizontal segments intersecting the stabbing line.
	 * The stabbing line is the ray to the right of stabbingRayLeftPt.
	 *
	 * @param stabbingRayLeftPt the left-hand origin of the stabbing line
	 * @return a List of DepthSegments intersecting the stabbing line
	 */
	vector<DepthSegment*>* findStabbedSegments(Coordinate &stabbingRayLeftPt);
	/**
	 * Finds all non-horizontal segments intersecting the stabbing line
	 * in the list of dirEdges.
	 * The stabbing line is the ray to the right of stabbingRayLeftPt.
	 *
	 * @param stabbingRayLeftPt the left-hand origin of the stabbing line
	 * @param stabbedSegments the current list of DepthSegments
	 *        intersecting the stabbing line
	 */
	void findStabbedSegments(Coordinate &stabbingRayLeftPt,vector<DirectedEdge*> *dirEdges,vector<DepthSegment*> *stabbedSegments);

	/**
	 * Finds all non-horizontal segments intersecting the stabbing line
	 * in the input dirEdge.
	 * The stabbing line is the ray to the right of stabbingRayLeftPt.
	 *
	 * @param stabbingRayLeftPt the left-hand origin of the stabbing line
	 * @param stabbedSegments the current list of {@link DepthSegments} intersecting the stabbing line
	 */
	void findStabbedSegments(Coordinate &stabbingRayLeftPt,DirectedEdge *dirEdge,vector<DepthSegment*> *stabbedSegments);
};

bool BufferSubgraphGT(BufferSubgraph *first, BufferSubgraph *second);

/*
 * \class BufferBuilder opBuffer.h geos/opBuffer.h
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
 */
class BufferBuilder {
friend class Unload;
public:
	/**
	 * Creates a new BufferBuilder
	 */
	BufferBuilder();
	~BufferBuilder();

	/**
	 * Sets the number of segments used to approximate a angle fillet
	 *
	 * @param quadrantSegments the number of segments in a fillet for
	 *  a quadrant
	 */
	void setQuadrantSegments(int nQuadrantSegments);

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
	void setWorkingPrecisionModel(PrecisionModel *pm);

	void setEndCapStyle(int nEndCapStyle);

	Geometry* buffer(const Geometry *g, double distance);
		// throw (GEOSException *);

private:
	/**
	 * Compute the change in depth as an edge is crossed from R to L
	 */
	static int depthDelta(Label *label);
	//static CGAlgorithms *cga;
	int quadrantSegments;
	int endCapStyle;
	PrecisionModel *workingPrecisionModel;
	const GeometryFactory *geomFact;
	EdgeList *edgeList;
	vector<Label *>newLabels;
	void computeNodedEdges(vector<SegmentString*> *bufferSegStrList, const PrecisionModel *precisionModel); // throw(GEOSException *);

	/**
	 * Inserted edges are checked to see if an identical edge already
	 * exists.
	 * If so, the edge is not inserted, but its label is merged
	 * with the existing edge.
	 */
	void insertEdge(Edge *e);
	vector<BufferSubgraph*>* createSubgraphs(PlanarGraph *graph);

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
	void buildSubgraphs(vector<BufferSubgraph*> *subgraphList, PolygonBuilder *polyBuilder);
};

} // namespace geos

#endif // ndef GEOS_OPBUFFER_H

/**********************************************************************
 * $Log$
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
 * Revision 1.10  2005/11/08 20:12:44  strk
 * Memory overhead reductions in buffer operations.
 *
 * Revision 1.9  2005/06/30 18:31:48  strk
 * Ported SubgraphDepthLocator optimizations from JTS code
 *
 * Revision 1.8  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.7  2005/02/04 18:49:48  strk
 * Changed ::computeDepths to use a set instead of a vector for checking
 * visited Edges.
 *
 * Revision 1.6  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.5  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.4  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.3  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.2  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.22  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.21  2004/06/30 20:59:13  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.20  2004/05/27 08:37:16  strk
 * Fixed a bug preventing OffsetCurveBuilder point list from being reset.
 *
 * Revision 1.19  2004/05/26 09:49:03  strk
 * PlanarGraph made local to ::buffer instead of Class private.
 *
 * Revision 1.18  2004/05/07 07:57:27  strk
 * Added missing EdgeNodingValidator to build scripts.
 * Changed SegmentString constructor back to its original form
 * (takes const void *), implemented local tracking of "contexts"
 * in caller objects for proper destruction.
 *
 * Revision 1.17  2004/05/05 16:57:48  strk
 * Rewritten static cga allocation to avoid copy constructor calls.
 *
 * Revision 1.16  2004/05/05 10:54:48  strk
 * Removed some private static heap explicit allocation, less cleanup done by
 * the unloader.
 *
 * Revision 1.15  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.14  2004/04/30 09:15:28  strk
 * Enlarged exception specifications to allow for AssertionFailedException.
 * Added missing initializers.
 *
 * Revision 1.13  2004/04/23 00:02:18  strk
 * const-correctness changes
 *
 * Revision 1.12  2004/04/20 10:58:04  strk
 * More memory leaks removed.
 *
 * Revision 1.11  2004/04/19 15:14:45  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.10  2004/04/19 12:51:01  strk
 * Memory leaks fixes. Throw specifications added.
 *
 * Revision 1.9  2004/04/15 14:00:30  strk
 * Added new cleanup to Unload::Release
 *
 * Revision 1.8  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.7  2004/03/01 22:04:59  strk
 * applied const correctness changes by Manuel Prieto Villegas <ManuelPrietoVillegas@telefonica.net>
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.5  2003/11/06 18:46:34  strk
 * Added throw specification for BufferOp's ::buildSubgraphs() 
 * and ::computeBuffer()
 *
 **********************************************************************/

