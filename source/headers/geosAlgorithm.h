#ifndef GEOS_ALGORITHM_H
#define GEOS_ALGORITHM_H

#include <memory>
#include "geom.h"
#include "util.h"
#include "platform.h"
#include "indexBintree.h"
#include "indexStrtree.h"
#include "indexStrtree.h"
#include "indexChain.h"

namespace geos {

class Coordinate;

class NotRepresentableException: public GEOSException {
public:
	NotRepresentableException();
	NotRepresentableException(string msg);
	~NotRepresentableException();
};

class PointInRing{
public:
	virtual ~PointInRing(){};
	virtual bool isInside(Coordinate& pt)=0;
};

class CGAlgorithms {
public:
	enum {
		CLOCKWISE=-1,
		COLLINEAR,
		COUNTERCLOCKWISE
	};
	CGAlgorithms(){};
	/**
	* Test whether a point lies inside a simple polygon (ring).
	* The ring may be oriented in either direction.
	* If the point lies on the ring boundary the result of this method is unspecified.
	*
	* @return true if the point lies in the interior of the ring
	*/
	virtual bool isPointInRing(Coordinate& p,CoordinateList* ring)=0;
	/**
	* Test whether a point lies on a linestring.
	*
	* @return true true if
	* the point is a vertex of the line or lies in the interior of a line
	* segment in the linestring
	*/
	virtual bool isOnLine(Coordinate& p,CoordinateList* linestring)=0;
	/**
	* Test whether a ring (simple polygon) is oriented counter-clockwise.
	*
	* @return true if the ring is oriented counter-clockwise
	*/
	virtual bool isCCW(CoordinateList* ring)=0;
	/**
	* Computes the orientation of a point q to the directed line segment p1-p2.
	* The orientation of a point relative to a directed line segment indicates
	* which way you turn to get to q after travelling from p1 to p2.
	*
	* @return 1 if q is counter-clockwise from p1-p2
	* @return -1 if q is clockwise from p1-p2
	* @return 0 if q is collinear with p1-p2
	*/
	virtual int computeOrientation(Coordinate& p1,Coordinate& p2,Coordinate& q)=0;
	static double distancePointLine(Coordinate& p,Coordinate& A,Coordinate& B);
	static double distanceLineLine(Coordinate& A,Coordinate& B,Coordinate& C,Coordinate& D);
	static double signedArea(CoordinateList* ring);
	static double length(CoordinateList* pts);
};

class HCoordinate {
public:
	static Coordinate* intersection(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2);
	double x,y,w;
	HCoordinate();
	HCoordinate(double _x, double _y, double _w);
	HCoordinate(Coordinate& p);
	HCoordinate(HCoordinate p1, HCoordinate p2);
	double getX();
	double getY();
	Coordinate* getCoordinate();
};

class SimplePointInRing: public PointInRing {
public:
	SimplePointInRing(LinearRing *ring);
	virtual ~SimplePointInRing();
	bool isInside(Coordinate& pt);
private:
	CGAlgorithms *cga;
	CoordinateList* pts;
};

class LineIntersector{
public:	
	static double computeEdgeDistance(Coordinate& p,Coordinate& p0,Coordinate& p1);
	static double nonRobustComputeEdgeDistance(Coordinate& p,Coordinate& p1,Coordinate& p2);
	LineIntersector();
	virtual ~LineIntersector();
	virtual void setMakePrecise(PrecisionModel *newPM);
	/**
	* Compute the intersection of a point p and the line p1-p2
	*/
	virtual void computeIntersection(Coordinate& p,Coordinate& p1,Coordinate& p2)=0;
	enum {
		DONT_INTERSECT,
		DO_INTERSECT,
		COLLINEAR
	};
	virtual void computeIntersection(Coordinate& p1,Coordinate& p2,Coordinate& p3, Coordinate& p4);
	virtual string toString();
	virtual bool hasIntersection();
	virtual int getIntersectionNum();
	virtual Coordinate& getIntersection(int intIndex);
	virtual bool isSameSignAndNonZero(double a,double b);
	virtual bool isIntersection(Coordinate& pt);
	virtual bool isProper();
	virtual Coordinate& getIntersectionAlongSegment(int segmentIndex,int intIndex);
	virtual int getIndexAlongSegment(int segmentIndex,int intIndex);
	virtual double getEdgeDistance(int geomIndex,int intIndex);
protected:
	/**
	* If makePrecise is true, computed intersection coordinates will be made precise
	* using Coordinate#makePrecise
	*/
	PrecisionModel *precisionModel;
	int result;
	Coordinate inputLines[2][2];
	Coordinate intPt[2];
	/**
	* The indexes of the endpoints of the intersection lines, in order along
	* the corresponding line
	*/
	int intLineIndex[2][2];
	bool isProperVar;
	Coordinate pa;
	Coordinate pb;
	virtual bool isCollinear();
	virtual int computeIntersect(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2)=0;
	virtual bool isEndPoint();
	virtual void computeIntLineIndex();
	virtual void computeIntLineIndex(int segmentIndex);
};

class RobustDeterminant {
public:
	static int signOfDet2x2(double x1,double y1,double x2,double y2);
};

class RobustLineIntersector: public LineIntersector {
public:
	RobustLineIntersector();
	virtual ~RobustLineIntersector();
	void computeIntersection(Coordinate& p,Coordinate& p1,Coordinate& p2);
	int computeIntersect(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2);
private:
//	bool between(Coordinate& p1,Coordinate& p2,Coordinate& q);
	int computeCollinearIntersection(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2);
	Coordinate* intersection(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2);
	void normalize(Coordinate *n1,Coordinate *n2,Coordinate *n3,Coordinate *n4,Coordinate *normPt);
	double smallestInAbsValue(double x1,double x2,double x3,double x4);
};

class NonRobustLineIntersector: public LineIntersector {
public:
	NonRobustLineIntersector();
	void computeIntersection(Coordinate& p,Coordinate& p1,Coordinate& p2);
protected:
	int computeIntersect(Coordinate& p1,Coordinate& p2,Coordinate& p3,Coordinate& p4);
private:
	int computeCollinearIntersection(Coordinate& p1,Coordinate& p2,Coordinate& p3,Coordinate& p4);
	double rParameter(Coordinate& p1,Coordinate& p2,Coordinate& p);
};

class RobustCGAlgorithms: public CGAlgorithms {
public:
	static int orientationIndex(Coordinate& p1,Coordinate& p2,Coordinate& q);
	RobustCGAlgorithms();
	~RobustCGAlgorithms();
	bool isCCW(CoordinateList* ring);
	bool isPointInRing(Coordinate& p,CoordinateList* ring);
	bool isOnLine(Coordinate& p,CoordinateList* pt);
	int computeOrientation(Coordinate& p1,Coordinate& p2,Coordinate& q);
private:
	bool isInEnvelope(Coordinate& p,CoordinateList* ring);
};

class NonRobustCGAlgorithms: public CGAlgorithms {
public:
	NonRobustCGAlgorithms();
	~NonRobustCGAlgorithms();
	bool isPointInRing(Coordinate& p,CoordinateList* ring);
	bool isOnLine(Coordinate& p,CoordinateList* pt);
	bool isCCW(CoordinateList* ring);
	int computeOrientation(Coordinate& p1,Coordinate& p2,Coordinate& q);
protected:
	LineIntersector *li;
};

class SimplePointInAreaLocator {
public:
	static int locate(Coordinate& p,Geometry *geom);
	static bool containsPointInPolygon(Coordinate& p,Polygon *poly);
private:
	static bool containsPoint(Coordinate& p,Geometry *geom);
};

/**
 * Computes the topological relationship ({@link Location})
 * of a single point to a Geometry.
 * The algorithm obeys the SFS boundaryDetermination rule to correctly determine
 * whether the point lies on the boundary or not.
 * Note that instances of this class are not reentrant.
 * @version 1.3
 */
class PointLocator {
public:
	PointLocator();
	~PointLocator();
	int locate(Coordinate& p,Geometry *geom);
	bool intersects(Coordinate& p,Geometry *geom);
	int locate(Coordinate& p,LineString *l);
	int locate(Coordinate& p,LinearRing *ring);
	int locate(Coordinate& p,Polygon *poly);
private:
	CGAlgorithms *cga;
	bool isIn;         // true if the point lies in or on any Geometry element
	int numBoundaries;    // the number of sub-elements whose boundaries the point lies in
	void computeLocation(Coordinate& p,Geometry *geom);
	void updateLocationInfo(int loc);
};


class MCPointInRing: public PointInRing {
public:
	MCPointInRing(LinearRing *newRing);
	virtual ~MCPointInRing();
	bool isInside(Coordinate& pt);
	void testLineSegment(Coordinate& p,LineSegment *seg);
	class MCSelecter: public MonotoneChainSelectAction {
	private:
		Coordinate p;
		MCPointInRing *parent;
	public:
		MCSelecter(Coordinate& newP,MCPointInRing *prt);
	    void select(LineSegment *ls);
	};
private:
	LinearRing *ring;
	BinTreeInterval *interval;
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
	void testLineSegment(Coordinate& p,LineSegment *seg);
public:
	SIRtreePointInRing(LinearRing *newRing);
	bool isInside(Coordinate& pt);
};

class CentroidPoint {
private:
	int ptCount;
	Coordinate* centSum;
public:
	CentroidPoint();
	virtual ~CentroidPoint();
	void add(Geometry *geom);
	void add(Coordinate *pt);
	Coordinate* getCentroid();
};

class CentroidLine {
private:
	Coordinate* centSum;
	double totalLength;
public:
	CentroidLine();
	virtual ~CentroidLine();
	void add(Geometry *geom);
	void add(CoordinateList *pts);
	Coordinate* getCentroid();
};

/**
 * Computes the centroid of an area geometry.
 * <h2>Algorithm</h2>
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
	void add(Geometry *geom);
	void add(CoordinateList *ring);
	Coordinate* getCentroid();
private:
	CGAlgorithms *cga;
	Coordinate* basePt;// the point all triangles are based at
	Coordinate* triangleCent3;// temporary variable to hold centroid of triangle
	double areasum2;        /* Partial area sum */
	Coordinate* cg3; // partial centroid sum
	void setBasePoint(Coordinate *newbasePt);
	void add(Polygon *poly);
	void addShell(CoordinateList *pts);
	void addHole(CoordinateList *pts);
	inline void addTriangle(Coordinate &p0,Coordinate &p1,Coordinate &p2,bool isPositiveArea);
	static inline  void centroid3(Coordinate &p1,Coordinate &p2,Coordinate &p3,Coordinate *c);
	static inline double area2(Coordinate &p1,Coordinate &p2,Coordinate &p3);
};

/**
 * Computes a point in the interior of an point geometry.
 * <h2>Algorithm</h2>
 * Find a point which is closest to the centroid of the geometry.
 */
class InteriorPointPoint {
private:
	Coordinate* centroid;
	double minDistance;
	Coordinate* interiorPoint;
	void add(Geometry *geom);
	void add(Coordinate *point);
public:
	InteriorPointPoint(Geometry *g);
	virtual	~InteriorPointPoint();
	Coordinate* getInteriorPoint();
};

/**
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
	Coordinate* getInteriorPoint();
private:
	Coordinate *centroid;
	double minDistance;
	Coordinate *interiorPoint;
	void addInterior(Geometry *geom);
	void addInterior(CoordinateList *pts);
	void addEndpoints(Geometry *geom);
	void addEndpoints(CoordinateList *pts);
	void add(Coordinate *point);
};

/**
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
	GeometryFactory *factory;
	Coordinate *interiorPoint;
	double maxWidth;
	void add(Geometry *geom);
public:
	InteriorPointArea(Geometry *g);
	virtual ~InteriorPointArea();
	Coordinate* getInteriorPoint();
	void addPolygon(Geometry *geometry);
	Coordinate* centre(Envelope *envelope);
protected:
	Geometry *widestGeometry(Geometry *geometry);
	Geometry *widestGeometry(GeometryCollection *gc);
	LineString *horizontalBisector(Geometry *geometry);

};

class BigQuad {
public:
	Coordinate northmost;
	Coordinate southmost;
	Coordinate westmost;
	Coordinate eastmost;
};

class ConvexHull {
private:
	PointLocator *pointLocator;
	CGAlgorithms *cgAlgorithms;
	Geometry *geometry;
	CoordinateList* reduce(CoordinateList *pts);
	CoordinateList* preSort(CoordinateList *pts);
	CoordinateList* grahamScan(CoordinateList *c);
	void radialSort(CoordinateList *p);
	int polarCompare(Coordinate o, Coordinate p, Coordinate q);
	bool isBetween(Coordinate c1, Coordinate c2, Coordinate c3);
    BigQuad* makeBigQuad(CoordinateList *pts);
	Geometry* lineOrPolygon(CoordinateList *newCoordinates);
	CoordinateList* cleanRing(CoordinateList *original);
public:
	ConvexHull(CGAlgorithms *newCgAlgorithms);
	~ConvexHull();
	Geometry* getConvexHull(Geometry *newGeometry);
};
}
#endif
