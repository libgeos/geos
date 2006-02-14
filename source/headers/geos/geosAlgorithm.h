/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_ALGORITHM_H
#define GEOS_ALGORITHM_H

#include <memory>
#include <geos/geom.h>
#include <geos/util.h>
#include <geos/platform.h>
#include <geos/indexBintree.h>
#include <geos/indexStrtree.h>
#include <geos/indexStrtree.h>
#include <geos/indexChain.h>

namespace geos {

class Coordinate;

/*
 * \class NotRepresentableException geosAlgorithm.h geos/geosAlgorithm.h
 * \brief
 * Indicates that a HCoordinate has been computed which is
 * not representable on the Cartesian plane.
 *
 * @version 1.4
 * @see HCoordinate
 */
class NotRepresentableException: public GEOSException {
public:
	NotRepresentableException();
	NotRepresentableException(string msg);
	~NotRepresentableException() throw() {}
};

class PointInRing{
public:
	virtual ~PointInRing(){};
	virtual bool isInside(const Coordinate& pt)=0;
};

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

/*
 * A LineIntersector is an algorithm that can both test whether
 * two line segments intersect and compute the intersection point
 * if they do.
 * The intersection point may be computed in a precise or non-precise manner.
 * Computing it precisely involves rounding it to an integer.  (This assumes
 * that the input coordinates have been made precise by scaling them to
 * an integer grid.)
 *
 */
class LineIntersector {
public:	
	// Return a Z value being the interpolation of Z from p0 and p1 at
	// the given point p
	static double interpolateZ(const Coordinate &p, const Coordinate &p0, const Coordinate &p1);
	static double computeEdgeDistance(const Coordinate& p, const Coordinate& p0, const Coordinate& p1);
	static double nonRobustComputeEdgeDistance(const Coordinate& p,const Coordinate& p1,const Coordinate& p2);

	LineIntersector(const PrecisionModel* initialPrecisionModel=NULL);

	~LineIntersector();

	/*
	 * Tests whether either intersection point is an interior point of
	 * one of the input segments.
	 *
	 * @return <code>true</code> if either intersection point is in
	 * the interior of one of the input segments
	 */
	bool isInteriorIntersection();

	/*
	 * Tests whether either intersection point is an interior point
	 * of the specified input segment.
	 *
	 * @return <code>true</code> if either intersection point is in
	 * the interior of the input segment
	 */
	bool isInteriorIntersection(int inputLineIndex);

	void setMakePrecise(const PrecisionModel *newPM);

	void setPrecisionModel(const PrecisionModel *newPM);

	/*
	 * Compute the intersection of a point p and the line p1-p2.
	 * This function computes the boolean value of the hasIntersection test.
	 * The actual value of the intersection (if there is one)
	 * is equal to the value of <code>p</code>.
	 */
	void computeIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2);

	/*
	 * Same as above but doen's compute intersection point. Faster.
	 */
	static bool hasIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2);


	enum {
		DONT_INTERSECT,
		DO_INTERSECT,
		COLLINEAR
	};

	void computeIntersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& p3, const Coordinate& p4);


	string toString() const;

	/**
	 * Tests whether the input geometries intersect.
	 *
	 * @return true if the input geometries intersect
	 */
	bool hasIntersection() const { return result!=DONT_INTERSECT; }

	int getIntersectionNum() const;

	const Coordinate& getIntersection(int intIndex) const;

	static bool isSameSignAndNonZero(double a,double b);

	bool isIntersection(const Coordinate& pt) const;
	bool isProper() const;
	const Coordinate& getIntersectionAlongSegment(int segmentIndex,int intIndex);
	int getIndexAlongSegment(int segmentIndex,int intIndex);
	double getEdgeDistance(int geomIndex,int intIndex) const;

private:

	/**
	 * If makePrecise is true, computed intersection coordinates
	 * will be made precise using Coordinate#makePrecise
	 */
	const PrecisionModel *precisionModel;
	int result;
	const Coordinate *inputLines[2][2];

	/*
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
	bool isCollinear() const;
	int computeIntersect(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2);
	bool isEndPoint() const;
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
	class MCSelecter: public MonotoneChainSelectAction {
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
	BinTreeInterval *interval;
	CoordinateSequence *pts;
	Bintree *tree;
	int crossings;  // number of segment/ray crossings
	void buildIndex();
	void testMonotoneChain(Envelope *rayEnv,MCSelecter *mcSelecter,indexMonotoneChain *mc);
};

class SIRtreePointInRing: public PointInRing {
private:
	LinearRing *ring;
	SIRtree *sirTree;
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
	Coordinate* centSum;
public:
	CentroidPoint();
	virtual ~CentroidPoint();
	void add(const Geometry *geom);
	void add(const Coordinate *pt);
	Coordinate* getCentroid() const;
};

class CentroidLine {
private:
	Coordinate centSum;
	double totalLength;
public:
	CentroidLine();
	~CentroidLine();
	void add(const Geometry *geom);
	void add(const CoordinateSequence *pts);
	Coordinate* getCentroid() const;
};

/*
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
	CentroidArea();
	virtual ~CentroidArea();
	void add(const Geometry *geom);
	void add(const CoordinateSequence *ring);
	Coordinate* getCentroid() const;
private:
	//CGAlgorithms *cga;
	Coordinate basePt;// the point all triangles are based at
	Coordinate triangleCent3;// temporary variable to hold centroid of triangle
	double areasum2;        /* Partial area sum */
	Coordinate cg3; // partial centroid sum
	void setBasePoint(const Coordinate &newbasePt);
	void add(const Polygon *poly);
	void addShell(const CoordinateSequence *pts);
	void addHole(const CoordinateSequence *pts);
	void addTriangle(const Coordinate &p0, const Coordinate &p1, const Coordinate &p2,bool isPositiveArea);
	static void centroid3(const Coordinate &p1, const Coordinate &p2, const Coordinate &p3, Coordinate &c);
	static double area2(const Coordinate &p1,const Coordinate &p2,const Coordinate &p3);
};

/*
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
	const Coordinate* centroid;
	double minDistance;
	Coordinate *interiorPoint;
	void add(const Geometry *geom);
	void add(const Coordinate *point);
public:
	InteriorPointPoint(const Geometry *g);
	virtual	~InteriorPointPoint();
	Coordinate* getInteriorPoint() const;
};

/*
 * Computes a point in the interior of an linear geometry.
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
	virtual ~InteriorPointLine();
	Coordinate* getInteriorPoint() const;
private:
	const Coordinate *centroid;
	double minDistance;
	Coordinate *interiorPoint;
	void addInterior(const Geometry *geom);
	void addInterior(const CoordinateSequence *pts);
	void addEndpoints(const Geometry *geom);
	void addEndpoints(const CoordinateSequence *pts);
	void add(const Coordinate *point);
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
	static double avg(double a, double b);
	const GeometryFactory *factory;
	Coordinate *interiorPoint;
	double maxWidth;
	void add(const Geometry *geom);
public:
	InteriorPointArea(const Geometry *g);
	virtual ~InteriorPointArea();
	Coordinate* getInteriorPoint() const;
	void addPolygon(const Geometry *geometry);
	Coordinate* centre(const Envelope *envelope) const;
protected:
	const Geometry *widestGeometry(const Geometry *geometry);
	const Geometry *widestGeometry(const GeometryCollection *gc);
	LineString *horizontalBisector(const Geometry *geometry);

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
		UniqueCoordinateArrayFilter filter(inputPts);
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

} // namespace geos

#endif

/**********************************************************************
 * $Log$
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

