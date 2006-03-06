/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_H
#define GEOS_ALGORITHM_H

#include <geos/geom.h>
#include <geos/util.h>
#include <geos/platform.h>
#include <geos/indexBintree.h>
#include <geos/indexStrtree.h>
#include <geos/indexStrtree.h>
#include <geos/indexChain.h>
#include <memory>
#include <string>

namespace geos {

/** \brief
 * Contains classes and interfaces implementing fundamental computational geometry algorithms.
 * 
 * <H3>Robustness</H3>
 * 
 * Geometrical algorithms involve a combination of combinatorial and numerical computation.  As with
 * all numerical computation using finite-precision numbers, the algorithms chosen are susceptible to
 * problems of robustness.  A robustness problem occurs when a numerical calculation produces an
 * incorrect answer for some inputs due to round-off errors.  Robustness problems are especially
 * serious in geometric computation, since they can result in errors during topology building.
 * <P>
 * There are many approaches to dealing with the problem of robustness in geometrical computation.
 * Not surprisingly, most robust algorithms are substantially more complex and less performant than
 * the non-robust versions.  Fortunately, JTS is sensitive to robustness problems in only a few key
 * functions (such as line intersection and the point-in-polygon test).  There are efficient robust
 * algorithms available for these functions, and these algorithms are implemented in JTS.
 * 
 * <H3>Computational Performance</H3>
 * 
 * Runtime performance is an important consideration for a production-quality implementation of
 * geometric algorithms.  The most computationally intensive algorithm used in JTS is intersection
 * detection.  JTS methods need to determine both all intersection between the line segments in a
 * single Geometry (self-intersection) and all intersections between the line segments of two different
 * Geometries.
 * <P>
 * The obvious naive algorithm for intersection detection (comparing every segment with every other)
 * has unacceptably slow performance.  There is a large literature of faster algorithms for intersection
 * detection.  Unfortunately, many of them involve substantial code complexity.  JTS tries to balance code
 * simplicity with performance gains.  It uses some simple techniques to produce substantial performance
 * gains for common types of input data.
 * 
 * 
 * <h2>Package Specification</h2>
 * 
 * <ul>
 *   <li>Java Topology Suite Technical Specifications
 *   <li><A HREF="http://www.opengis.org/techno/specs.htm">
 *       OpenGIS Simple Features Specification for SQL</A>
 * </ul>
 * 
 */
namespace algorithm { // geos.algorithm


/**
 * \class NotRepresentableException geosAlgorithm.h geos/geosAlgorithm.h
 * \brief
 * Indicates that a HCoordinate has been computed which is
 * not representable on the Cartesian plane.
 *
 * @version 1.4
 * @see HCoordinate
 */
class NotRepresentableException: public util::GEOSException {
public:
	NotRepresentableException();
	NotRepresentableException(std::string msg);
	~NotRepresentableException() throw() {}
};

class PointInRing{
public:
	virtual ~PointInRing(){};
	virtual bool isInside(const Coordinate& pt)=0;
};

/**
 * \brief
 * Specifies and implements various fundamental Computational Geometric algorithms.
 * The algorithms supplied in this class are robust for double-precision floating point.
 *
 */
class CGAlgorithms {
public:
	enum {
		CLOCKWISE=-1,
		COLLINEAR,
		COUNTERCLOCKWISE
	};
	enum {
		RIGHT=-1,
		LEFT,
		STRAIGHT
	};
	CGAlgorithms(){};

	/**
	 * Test whether a point lies inside a ring.
	 * The ring may be oriented in either direction.
	 * If the point lies on the ring boundary the result
	 * of this method is unspecified.
	 * 
	 * This algorithm does not attempt to first check the
	 *point against the envelope of the ring.
	 *
	 * @param p point to check for ring inclusion
	 * @param ring assumed to have first point identical to last point
	 * @return <code>true</code> if p is inside ring
	 */
	static bool isPointInRing(const Coordinate& p, const CoordinateSequence* ring);

	/// Same as above, but taking a Coordinate::ConstVect (faster)
	static bool isPointInRing(const Coordinate& p, const Coordinate::ConstVect& ring);

	/**
	 * Test whether a point lies on a linestring.
	 *
	 * @return true true if
	 * the point is a vertex of the line or lies in the interior of a line
	 * segment in the linestring
	 */
	static bool isOnLine(const Coordinate& p, const CoordinateSequence* pt);

	/*
	 * Computes whether a ring defined by an array of Coordinate is
	 * oriented counter-clockwise.
	 * 
	 *  - The list of points is assumed to have the first and last
	 *    points equal.
	 *  - This will handle coordinate lists which contain repeated points.
	 *  - If the ring is invalid, the answer returned may not be correct.
	 * 
	 *
	 * @param ring an array of coordinates forming a ring
	 * @return <code>true</code> if the ring is oriented counter-clockwise.
	 */
	static bool isCCW(const CoordinateSequence* ring);

	/**
	* Computes the orientation of a point q to the directed line segment p1-p2.
	* The orientation of a point relative to a directed line segment indicates
	* which way you turn to get to q after travelling from p1 to p2.
	*
	* @return 1 if q is counter-clockwise from p1-p2
	* @return -1 if q is clockwise from p1-p2
	* @return 0 if q is collinear with p1-p2
	*/
	static int computeOrientation(const Coordinate& p1, const Coordinate& p2, const Coordinate& q);
	static double distancePointLine(const Coordinate& p,const Coordinate& A,const Coordinate& B);
	/**
	* Computes the perpendicular distance from a point p
	* to the (infinite) line containing the points AB
	*
	* @param p the point to compute the distance for
	* @param A one point of the line
	* @param B another point of the line (must be different to A)
	* @return the distance from p to line AB
	*/
	static double distancePointLinePerpendicular(const Coordinate& p,const Coordinate& A,const Coordinate& B);
	static double distanceLineLine(const Coordinate& A, const Coordinate& B, const Coordinate& C, const Coordinate& D);
	static double signedArea(const CoordinateSequence* ring);
	/**
	* Computes the length of a linestring specified by a sequence of points.
	*
	* @param pts the points specifying the linestring
	* @return the length of the linestring
	*/
	static double length(const CoordinateSequence* pts);
	/**
	* Returns the index of the direction of the point <code>q</code>
	* relative to a
	* vector specified by <code>p1-p2</code>.
	*
	* @param p1 the origin point of the vector
	* @param p2 the final point of the vector
	* @param q the point to compute the direction to
	*
	* @return 1 if q is counter-clockwise (left) from p1-p2
	* @return -1 if q is clockwise (right) from p1-p2
	* @return 0 if q is collinear with p1-p2
	*/
	static int orientationIndex(const Coordinate& p1,const Coordinate& p2,const Coordinate& q);

};

/// Represents a homogeneous coordinate for 2-D coordinates.
class HCoordinate {
public:

	static void intersection(const Coordinate &p1, const Coordinate &p2,
		const Coordinate &q1, const Coordinate &q2, Coordinate &ret);

	double x,y,w;
	HCoordinate();
	HCoordinate(double _x, double _y, double _w);
	HCoordinate(const Coordinate& p);
	HCoordinate(const HCoordinate &p1, const HCoordinate &p2);
	double getX() const;
	double getY() const;
	void getCoordinate(Coordinate &ret) const;
};

class SimplePointInRing: public PointInRing {
public:
	SimplePointInRing(LinearRing *ring);
	virtual ~SimplePointInRing();
	bool isInside(const Coordinate& pt);
private:
	const CoordinateSequence* pts;
};

/** \brief
 * A LineIntersector is an algorithm that can both test whether
 * two line segments intersect and compute the intersection point
 * if they do.
 *
 * The intersection point may be computed in a precise or non-precise manner.
 * Computing it precisely involves rounding it to an integer.  (This assumes
 * that the input coordinates have been made precise by scaling them to
 * an integer grid.)
 *
 */
class LineIntersector {
public:	

	/// \brief
	/// Return a Z value being the interpolation of Z from p0 and p1 at
	/// the given point p
	static double interpolateZ(const Coordinate &p, const Coordinate &p0, const Coordinate &p1);


	/// Computes the "edge distance" of an intersection point p in an edge.
	//
	/// The edge distance is a metric of the point along the edge.
	/// The metric used is a robust and easy to compute metric function.
	/// It is <b>not</b> equivalent to the usual Euclidean metric.
	/// It relies on the fact that either the x or the y ordinates of the
	/// points in the edge are unique, depending on whether the edge is longer in
	/// the horizontal or vertical direction.
	/// 
	/// NOTE: This function may produce incorrect distances
	///  for inputs where p is not precisely on p1-p2
	/// (E.g. p = (139,9) p1 = (139,10), p2 = (280,1) produces distanct
	/// 0.0, which is incorrect.
	/// 
	/// My hypothesis is that the function is safe to use for points which are the
	/// result of <b>rounding</b> points which lie on the line,
	/// but not safe to use for <b>truncated</b> points.
	///
	static double computeEdgeDistance(const Coordinate& p, const Coordinate& p0, const Coordinate& p1);

	static double nonRobustComputeEdgeDistance(const Coordinate& p,const Coordinate& p1,const Coordinate& p2);

	LineIntersector(const PrecisionModel* initialPrecisionModel=NULL)
		:
		precisionModel(initialPrecisionModel),
		result(0)
	{}

	~LineIntersector() {}

	/** \brief
	 * Tests whether either intersection point is an interior point of
	 * one of the input segments.
	 *
	 * @return <code>true</code> if either intersection point is in
	 * the interior of one of the input segments
	 */
	bool isInteriorIntersection();

	/** \brief
	 * Tests whether either intersection point is an interior point
	 * of the specified input segment.
	 *
	 * @return <code>true</code> if either intersection point is in
	 * the interior of the input segment
	 */
	bool isInteriorIntersection(int inputLineIndex);

	/// Force computed intersection to be rounded to a given precision model.
	//
	/// No getter is provided, because the precision model is not required
	/// to be specified.
	/// @param precisionModel the PrecisionModel to use for rounding
	///
	void setPrecisionModel(const PrecisionModel *newPM) {
		precisionModel=newPM;
	}

	/// Compute the intersection of a point p and the line p1-p2.
	//
	/// This function computes the boolean value of the hasIntersection test.
	/// The actual value of the intersection (if there is one)
	/// is equal to the value of <code>p</code>.
	///
	void computeIntersection(const Coordinate& p, const Coordinate& p1, const Coordinate& p2);

	/// Same as above but doen's compute intersection point. Faster.
	static bool hasIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2);

	enum {
		DONT_INTERSECT,
		DO_INTERSECT,
		COLLINEAR
	};

	/// Computes the intersection of the lines p1-p2 and p3-p4
	void computeIntersection(const Coordinate& p1, const Coordinate& p2,
			const Coordinate& p3, const Coordinate& p4);

	std::string toString() const;

	/**
	 * Tests whether the input geometries intersect.
	 *
	 * @return true if the input geometries intersect
	 */
	bool hasIntersection() const { return result!=DONT_INTERSECT; }

	/// Returns the number of intersection points found.
	//
	/// This will be either 0, 1 or 2.
	///
	int getIntersectionNum() const { return result; }

	
	/// Returns the intIndex'th intersection point
	//
	/// @param intIndex is 0 or 1
	///
	/// @return the intIndex'th intersection point
	///
	const Coordinate& getIntersection(int intIndex) const {
		return intPt[intIndex];
	}

	/// Returns false if both numbers are zero.
	//
	/// @return true if both numbers are positive or if both numbers are negative.
	///
	static bool isSameSignAndNonZero(double a,double b);

	/** \brief
	 * Test whether a point is a intersection point of two line segments.
	 *
	 * Note that if the intersection is a line segment, this method only tests for
	 * equality with the endpoints of the intersection segment.
	 * It does <b>not</b> return true if
	 * the input point is internal to the intersection segment.
	 *
	 * @return true if the input point is one of the intersection points.
	 */
	bool isIntersection(const Coordinate& pt) const;

	/** \brief
	 * Tests whether an intersection is proper.
	 * 
	 * The intersection between two line segments is considered proper if
	 * they intersect in a single point in the interior of both segments
	 * (e.g. the intersection is a single point and is not equal to any of the
	 * endpoints).
	 * 
	 * The intersection between a point and a line segment is considered proper
	 * if the point lies in the interior of the segment (e.g. is not equal to
	 * either of the endpoints).
	 *
	 * @return true if the intersection is proper
	 */
	bool isProper() const {
		return hasIntersection()&&isProperVar;
	}

	/** \brief
	 * Computes the intIndex'th intersection point in the direction of
	 * a specified input line segment
	 *
	 * @param segmentIndex is 0 or 1
	 * @param intIndex is 0 or 1
	 *
	 * @return the intIndex'th intersection point in the direction of the
	 *         specified input line segment
	 */
	const Coordinate& getIntersectionAlongSegment(int segmentIndex,int intIndex);

	/** \brief
	 * Computes the index of the intIndex'th intersection point in the direction of
	 * a specified input line segment
	 *
	 * @param segmentIndex is 0 or 1
	 * @param intIndex is 0 or 1
	 *
	 * @return the index of the intersection point along the segment (0 or 1)
	 */
	int getIndexAlongSegment(int segmentIndex,int intIndex);

	/** \brief
	 * Computes the "edge distance" of an intersection point along the specified
	 * input line segment.
	 *
	 * @param segmentIndex is 0 or 1
	 * @param intIndex is 0 or 1
	 *
	 * @return the edge distance of the intersection point
	 */
	double getEdgeDistance(int geomIndex,int intIndex) const;

private:

	/**
	 * If makePrecise is true, computed intersection coordinates
	 * will be made precise using Coordinate#makePrecise
	 */
	const PrecisionModel *precisionModel;

	int result;

	const Coordinate *inputLines[2][2];

	/**
	 * We store real Coordinates here because
	 * we must compute the Z of intersection point.
	 */
	Coordinate intPt[2];

	/**
	 * The indexes of the endpoints of the intersection lines, in order along
	 * the corresponding line
	 */
	int intLineIndex[2][2];

	bool isProperVar;
	//Coordinate &pa;
	//Coordinate &pb;

	bool isCollinear() const { return result==COLLINEAR; }

	int computeIntersect(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2);

	bool isEndPoint() const {
		return hasIntersection()&&!isProperVar;
	}

	void computeIntLineIndex();

	void computeIntLineIndex(int segmentIndex);

	int computeCollinearIntersection(const Coordinate& p1,
		const Coordinate& p2, const Coordinate& q1,
		const Coordinate& q2);

	void intersection(const Coordinate& p1, const Coordinate& p2,
		const Coordinate& q1, const Coordinate& q2,
		Coordinate &ret) const;

	double smallestInAbsValue(double x1, double x2,
		double x3, double x4) const;

	/**
	 * Test whether a point lies in the envelopes of both input segments.
	 * A correctly computed intersection point should return true
	 * for this test.
	 * Since this test is for debugging purposes only, no attempt is
	 * made to optimize the envelope test.
	 *
	 * @return true if the input point lies within both
	 *         input segment envelopes
	 */
	bool isInSegmentEnvelopes(const Coordinate& intPt);

	/**
	 * Normalize the supplied coordinates to
	 * so that the midpoint of their intersection envelope
	 * lies at the origin.
	 *
	 * @param n00
	 * @param n01
	 * @param n10
	 * @param n11
	 * @param normPt
	 */
	void normalizeToEnvCentre(Coordinate &n00, Coordinate &n01,
		Coordinate &n10, Coordinate &n11, Coordinate &normPt) const;
};

class RobustDeterminant {
public:
	static int signOfDet2x2(double x1,double y1,double x2,double y2);
};

class SimplePointInAreaLocator {
public:
	static int locate(const Coordinate& p, const Geometry *geom);
	static bool containsPointInPolygon(const Coordinate& p,const Polygon *poly);
private:
	static bool containsPoint(const Coordinate& p,const Geometry *geom);
};

/**
 * \class PointLocator geosAlgorithm.h geos/geosAlgorithm.h
 *
 * \brief
 * Computes the topological relationship (Location)
 * of a single point to a Geometry.
 *
 * The algorithm obeys the SFS boundaryDetermination rule to correctly determine
 * whether the point lies on the boundary or not.
 *
 * Notes:
 *	- instances of this class are not reentrant.
 *	- LinearRing objects do not enclose any area
 *	  points inside the ring are still in the EXTERIOR of the ring.
 *
 * Last port: algorithm/PointLocator.java rev. 1.26 (JTS-1.7+)
 */
class PointLocator {
public:
	PointLocator() {}
	~PointLocator() {}

	/**
	 * Computes the topological relationship (Location) of a single point
	 * to a Geometry.
	 * It handles both single-element
	 * and multi-element Geometries.
	 * The algorithm for multi-part Geometries
	 * takes into account the boundaryDetermination rule.
	 *
	 * @return the Location of the point relative to the input Geometry
	 */
	int locate(const Coordinate& p,const Geometry *geom);

	/**
	 * Convenience method to test a point for intersection with
	 * a Geometry
	 *
	 * @param p the coordinate to test
	 * @param geom the Geometry to test
	 * @return <code>true</code> if the point is in the interior or boundary of the Geometry
	 */
	bool intersects(const Coordinate& p, const Geometry *geom) {
		return locate(p,geom)!=Location::EXTERIOR;
	}

private:
	bool isIn;         // true if the point lies in or on any Geometry element
	int numBoundaries;    // the number of sub-elements whose boundaries the point lies in
	void computeLocation(const Coordinate& p,const Geometry *geom);
	void updateLocationInfo(int loc);
	int locate(const Coordinate& p,const LineString *l);
	int locateInPolygonRing(const Coordinate& p,const LinearRing *ring);
	int locate(const Coordinate& p,const Polygon *poly);

};


class MCPointInRing: public PointInRing {
public:
	MCPointInRing(LinearRing *newRing);
	virtual ~MCPointInRing();
	bool isInside(const Coordinate& pt);
	void testLineSegment(Coordinate& p,LineSegment *seg);
	class MCSelecter: public index::chain::MonotoneChainSelectAction {
	using MonotoneChainSelectAction::select;
	private:
		Coordinate p;
		MCPointInRing *parent;
	public:
		MCSelecter(const Coordinate& newP,MCPointInRing *prt);
		void select(LineSegment *ls);
	};
private:
	LinearRing *ring;
	index::bintree::BinTreeInterval *interval;
	CoordinateSequence *pts;
	index::bintree::Bintree *tree;
	int crossings;  // number of segment/ray crossings
	void buildIndex();
	void testMonotoneChain(Envelope *rayEnv,
			MCSelecter *mcSelecter,
			index::chain::MonotoneChain *mc);
};

class SIRtreePointInRing: public PointInRing {
private:
	LinearRing *ring;
	index::strtree::SIRtree *sirTree;
	int crossings;  // number of segment/ray crossings
	void buildIndex();
	void testLineSegment(const Coordinate& p,LineSegment *seg);
public:
	SIRtreePointInRing(LinearRing *newRing);
	bool isInside(const Coordinate& pt);
};

class CentroidPoint {

private:

	int ptCount;

	Coordinate centSum;

public:

	CentroidPoint()
		:
		ptCount(0),
		centSum(0.0, 0.0)
	{}

	~CentroidPoint()
	{}

	/**
	 * Adds the point(s) defined by a Geometry to the centroid total.
	 * If the geometry is not of dimension 0 it does not contribute to the
	 * centroid.
	 * @param geom the geometry to add
	 */
	void add(const Geometry *geom);

	void add(const Coordinate *pt);

	Coordinate* getCentroid() const;

	/// Return false if centroid could not be computed
	bool getCentroid(Coordinate& ret) const;
};

class CentroidLine {
private:

	Coordinate centSum;

	double totalLength;

public:

	CentroidLine()
		:
		centSum(0.0, 0.0),
		totalLength(0.0)
	{}

	~CentroidLine() {}

	/** \brief
	 * Adds the linestring(s) defined by a Geometry to the centroid total.
	 *
	 * If the geometry is not linear it does not contribute to the centroid
	 * @param geom the geometry to add
	 */
	void add(const Geometry *geom);

	/** \brief
	 * Adds the length defined by an array of coordinates.
	 *
	 * @param pts an array of {@link Coordinate}s
	 */
	void add(const CoordinateSequence *pts);

	Coordinate* getCentroid() const;

	/// return false if centroid could not be computed
	bool getCentroid(Coordinate& ret) const;
};

/**
 * \class CentroidArea geosAlgorithm.h geos/geosAlgorithm.h
 *
 * \brief Computes the centroid of an area geometry.
 *
 * Algorithm:
 *
 * Based on the usual algorithm for calculating
 * the centroid as a weighted sum of the centroids
 * of a decomposition of the area into (possibly overlapping) triangles.
 * The algorithm has been extended to handle holes and multi-polygons.
 * See <code>http://www.faqs.org/faqs/graphics/algorithms-faq/</code>
 * for further details of the basic approach.
 */
class CentroidArea {

public:

	CentroidArea()
		:
		basePt(0.0, 0.0),
		areasum2(0)
	{}

	~CentroidArea() {}

	/**
	 * Adds the area defined by a Geometry to the centroid total.
	 * If the geometry has no area it does not contribute to the centroid.
	 *
	 * @param geom the geometry to add
	 */
	void add(const Geometry *geom);

	/**
	 * Adds the area defined by an array of
	 * coordinates.  The array must be a ring;
	 * i.e. end with the same coordinate as it starts with.
	 * @param ring an array of {@link Coordinate}s
	 */
	void add(const CoordinateSequence *ring);

	Coordinate* getCentroid() const;

	/// Return false if a centroid couldn't be computed
	bool getCentroid(Coordinate& ret) const;

private:

	/// the point all triangles are based at
	Coordinate basePt;

	// temporary variable to hold centroid of triangle
	Coordinate triangleCent3;

	/// Partial area sum 
	double areasum2;       

	/// partial centroid sum
	Coordinate cg3;

	void setBasePoint(const Coordinate &newbasePt);

	void add(const Polygon *poly);

	void addShell(const CoordinateSequence *pts);

	void addHole(const CoordinateSequence *pts);
	
	void addTriangle(const Coordinate &p0, const Coordinate &p1,
			const Coordinate &p2,bool isPositiveArea);

	static void centroid3(const Coordinate &p1, const Coordinate &p2,
			const Coordinate &p3, Coordinate &c);

	static double area2(const Coordinate &p1, const Coordinate &p2,
			const Coordinate &p3);

};

/**
 * \class InteriorPointPoint geosAlgorithm.h geos/geosAlgorithm.h
 * \brief
 * Computes a point in the interior of an point geometry.
 *
 * Algorithm:
 *
 * Find a point which is closest to the centroid of the geometry.
 */
class InteriorPointPoint {
private:

	bool hasInterior;

	Coordinate centroid;

	double minDistance;

	Coordinate interiorPoint;

	/**
	 * Tests the point(s) defined by a Geometry for the best inside point.
	 * If a Geometry is not of dimension 0 it is not tested.
	 * @param geom the geometry to add
	 */
	void add(const Geometry *geom);

	void add(const Coordinate *point);

public:

	InteriorPointPoint(const Geometry *g);

	~InteriorPointPoint() {}

	bool getInteriorPoint(Coordinate& ret) const;

};

/** \brief
 * Computes a point in the interior of an linear geometry.
 *
 * <h2>Algorithm</h2>
 * <ul>
 * <li>Find an interior vertex which is closest to
 * the centroid of the linestring.
 * <li>If there is no interior vertex, find the endpoint which is
 * closest to the centroid.
 * </ul>
 */
class InteriorPointLine {
public:

	InteriorPointLine(Geometry *g);

	~InteriorPointLine();

	//Coordinate* getInteriorPoint() const;

	bool getInteriorPoint(Coordinate& ret) const;

private:

	bool hasInterior;

	Coordinate centroid;

	double minDistance;

	Coordinate interiorPoint;

	void addInterior(const Geometry *geom);

	void addInterior(const CoordinateSequence *pts);
	
	void addEndpoints(const Geometry *geom);

	void addEndpoints(const CoordinateSequence *pts);

	void add(const Coordinate& point);

};

/*
 * Computes a point in the interior of an area geometry.
 *
 * <h2>Algorithm</h2>
 * <ul>
 *   <li>Find the intersections between the geometry
 *       and the horizontal bisector of the area's envelope
 *   <li>Pick the midpoint of the largest intersection (the intersections
 *       will be lines and points)
 * </ul>
 *
 * <b>
 * Note: If a fixed precision model is used,
 * in some cases this method may return a point
 * which does not lie in the interior.
 * </b>
 */
class InteriorPointArea {

private:

	bool foundInterior;

	const GeometryFactory *factory;

	Coordinate interiorPoint;

	double maxWidth;

	void add(const Geometry *geom);

	const Geometry *widestGeometry(const Geometry *geometry);

	const Geometry *widestGeometry(const GeometryCollection *gc);

	LineString *horizontalBisector(const Geometry *geometry);

public:

	InteriorPointArea(const Geometry *g);

	~InteriorPointArea();

	bool getInteriorPoint(Coordinate& ret) const;

	/** \brief
	 * Finds a reasonable point at which to label a Geometry.
	 *
	 * @param geometry the geometry to analyze
	 * @return the midpoint of the largest intersection between the geometry and
	 * a line halfway down its envelope
	 */
	void addPolygon(const Geometry *geometry);

};

class BigQuad {
public:
	Coordinate northmost;
	Coordinate southmost;
	Coordinate westmost;
	Coordinate eastmost;
};


/**
 * Computes the convex hull of a Geometry.
 *
 * The convex hull is the smallest convex Geometry that contains all the
 * points in the input Geometry.
 * 
 * Uses the Graham Scan algorithm.
 *
 * Last port: algorithm/ConvexHull.java rev. 1.26 (JTS-1.7)
 *
 */
class ConvexHull {
private:
	const GeometryFactory *geomFactory;
	Coordinate::ConstVect inputPts;

	void extractCoordinates(const Geometry *geom)
	{
		util::UniqueCoordinateArrayFilter filter(inputPts);
		geom->apply_ro(&filter);
	}

	/// Create a CoordinateSequence from the Coordinate::ConstVect
	/// This is needed to construct the geometries.
	/// Here coordinate copies happen
	/// The returned object is newly allocated !NO EXCEPTION SAFE!
	CoordinateSequence *toCoordinateSequence(Coordinate::ConstVect &cv);

	void computeOctPts(const Coordinate::ConstVect &src,
			Coordinate::ConstVect &tgt);

	bool computeOctRing(const Coordinate::ConstVect &src,
			Coordinate::ConstVect &tgt);

	/**
	 * Uses a heuristic to reduce the number of points scanned
	 * to compute the hull.
	 * The heuristic is to find a polygon guaranteed to
	 * be in (or on) the hull, and eliminate all points inside it.
	 * A quadrilateral defined by the extremal points
	 * in the four orthogonal directions
	 * can be used, but even more inclusive is
	 * to use an octilateral defined by the points in the
	 * 8 cardinal directions.
	 *
	 * Note that even if the method used to determine the polygon
	 * vertices is not 100% robust, this does not affect the
	 * robustness of the convex hull.
	 *
	 * @param pts The vector of const Coordinate pointers
	 *            to be reduced
	 *
	 *
	 * WARNING: the parameter will be modified
	 *
	 */
	void reduce(Coordinate::ConstVect &pts);


	/// parameter will be modified
	void preSort(Coordinate::ConstVect &pts);

	/**
	 * Given two points p and q compare them with respect to their radial
	 * ordering about point o.  First checks radial ordering.
	 * If points are collinear, the comparison is based
	 * on their distance to the origin.
	 * 
	 * p < q iff
	 *
	 * - ang(o-p) < ang(o-q) (e.g. o-p-q is CCW)
	 * - or ang(o-p) == ang(o-q) && dist(o,p) < dist(o,q)
	 *
	 * @param o the origin
	 * @param p a point
	 * @param q another point
	 * @return -1, 0 or 1 depending on whether p is less than,
	 * equal to or greater than q
	 */
	int polarCompare(const Coordinate &o,
			const Coordinate &p, const Coordinate &q);

	void grahamScan(const Coordinate::ConstVect &c,
			Coordinate::ConstVect &ps);

	/**
	 * @param  vertices  the vertices of a linear ring,
	 *                   which may or may not be
	 *                   flattened (i.e. vertices collinear)
	 *
	 * @return           a 2-vertex LineString if the vertices are
	 *                   collinear; otherwise, a Polygon with unnecessary
	 *                   (collinear) vertices removed
	 */
	Geometry* lineOrPolygon(const Coordinate::ConstVect &vertices);

	/**
	 * Write in 'cleaned' a version of 'input' with collinear
	 * vertexes removed.
   	 */
	void cleanRing(const Coordinate::ConstVect &input,
			Coordinate::ConstVect &cleaned);

	/**
	 * @return  whether the three coordinates are collinear
	 *          and c2 lies between c1 and c3 inclusive
	 */
	bool isBetween(const Coordinate& c1, const Coordinate& c2, const Coordinate& c3);

#if 0
	void radialSort(CoordinateSequence *p);
	void makeBigQuad(const CoordinateSequence *pts, BigQuad &ret);
#endif

public:

	/**
	 * Create a new convex hull construction for the input Geometry.
	 */
	ConvexHull(const Geometry *newGeometry)
		:
		geomFactory(newGeometry->getFactory())
	{
		extractCoordinates(newGeometry);
	}


	~ConvexHull() {}

	/**
	 * Returns a Geometry that represents the convex hull of
	 * the input geometry.
	 * The returned geometry contains the minimal number of points
	 * needed to represent the convex hull. 
	 * In particular, no more than two consecutive points
	 * will be collinear.
	 *
	 * @return if the convex hull contains 3 or more points,
	 *         a Polygon; 2 points, a LineString;
	 *         1 point, a Point; 0 points, an empty GeometryCollection.
	 */
	Geometry* getConvexHull();
};


/*
 * Computes the minimum diameter of a {@link Geometry}.
 * The minimum diameter is defined to be the
 * width of the smallest band that
 * contains the geometry,
 * where a band is a strip of the plane defined
 * by two parallel lines.
 * This can be thought of as the smallest hole that the geometry can be
 * moved through, with a single rotation.
 * <p>
 * The first step in the algorithm is computing the convex hull of the Geometry.
 * If the input Geometry is known to be convex, a hint can be supplied to
 * avoid this computation.
 *
 * @see ConvexHull
 *
 */
class MinimumDiameter {
private:
	const Geometry* inputGeom;
	bool isConvex;
	LineSegment* minBaseSeg;
	Coordinate* minWidthPt;
	int minPtIndex;
	double minWidth;
	void computeMinimumDiameter();
	void computeWidthConvex(const Geometry* geom);
	/**
	 * Compute the width information for a ring of {@link Coordinate}s.
	 * Leaves the width information in the instance variables.
	 *
	 * @param pts
	 * @return
	 */
	void computeConvexRingMinDiameter(const CoordinateSequence *pts);

	unsigned int findMaxPerpDistance(const CoordinateSequence* pts,
		LineSegment* seg, unsigned int startIndex);

	static unsigned int getNextIndex(const CoordinateSequence* pts,
		unsigned int index);

public:
	~MinimumDiameter();

	/**
	* Compute a minimum diameter for a giver {@link Geometry}.
	*
	* @param geom a Geometry
	*/
	MinimumDiameter(const Geometry* newInputGeom);

	/**
	* Compute a minimum diameter for a giver {@link Geometry},
	* with a hint if
	* the Geometry is convex
	* (e.g. a convex Polygon or LinearRing,
	* or a two-point LineString, or a Point).
	*
	* @param geom a Geometry which is convex
	* @param isConvex <code>true</code> if the input geometry is convex
	*/
	MinimumDiameter(const Geometry* newInputGeom,const bool newIsConvex);

	/**
	* Gets the length of the minimum diameter of the input Geometry
	*
	* @return the length of the minimum diameter
	*/
	double getLength();

	/**
	* Gets the {@link Coordinate} forming one end of the minimum diameter
	*
	* @return a coordinate forming one end of the minimum diameter
	*/
	Coordinate* getWidthCoordinate();

	/**
	* Gets the segment forming the base of the minimum diameter
	*
	* @return the segment forming the base of the minimum diameter
	*/
	LineString* getSupportingSegment();

	/**
	* Gets a {@link LineString} which is a minimum diameter
	*
	* @return a {@link LineString} which is a minimum diameter
	*/
	LineString* getDiameter();
};

} // namespace geos.algorithm
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.28  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.27  2006/03/03 13:50:16  strk
 * Cleaned up InteriorPointLine class
 *
 * Revision 1.26  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.25  2006/03/01 17:16:39  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.24  2006/02/27 09:05:33  strk
 * Doxygen comments, a few inlines and general cleanups
 *
 * Revision 1.23  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.22  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.21  2006/02/14 13:28:25  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.20  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.19  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.18  2005/11/29 15:16:44  strk
 * Fixed sign-related warnings and signatures.
 *
 * Revision 1.17  2005/11/24 23:07:00  strk
 * CentroidLine made concrete class (only destructor was virtual) - avoided heap allocation for owned Coordinate centSum
 *
 * Revision 1.16  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.15  2005/11/15 18:30:59  strk
 * Removed dead code
 *
 * Revision 1.14  2005/11/10 15:20:32  strk
 * Made virtual overloads explicit.
 *
 * Revision 1.13  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.12  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.11  2005/05/09 10:35:20  strk
 * Ported JTS robustness patches made by Martin on suggestions by Kevin.
 *
 * Revision 1.10  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.9  2004/11/23 19:53:07  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.8  2004/11/06 08:16:46  strk
 * Fixed CGAlgorithms::isCCW from JTS port.
 * Code cleanup in IsValidOp.
 *
 * Revision 1.7  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
 *
 * Revision 1.6  2004/09/13 10:12:49  strk
 * Added invalid coordinates checks in IsValidOp.
 * Cleanups.
 *
 * Revision 1.5  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.4  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.3  2004/07/07 10:29:54  strk
 * Adjusted exceptions documentation.
 *
 * Revision 1.2  2004/07/07 09:38:12  strk
 * Dropped WKTWriter::stringOfChars (implemented by std::string).
 * Dropped WKTWriter default constructor (internally created GeometryFactory).
 * Updated XMLTester to respect the changes.
 * Main documentation page made nicer.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.32  2004/06/30 20:59:12  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.31  2004/04/20 12:47:57  strk
 * MinimumDiameter leaks plugged.
 *
 * Revision 1.30  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.29  2004/02/27 17:42:15  strk
 * made CGAlgorithms::signedArea() and CGAlgorithms::length() arguments const-correct
 *
 * Revision 1.28  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

