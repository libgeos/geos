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
	virtual bool isInside(const Coordinate& pt)=0;
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
	virtual bool isPointInRing(const Coordinate& p, const CoordinateList* ring) const=0;
	/**
	* Test whether a point lies on a linestring.
	*
	* @return true true if
	* the point is a vertex of the line or lies in the interior of a line
	* segment in the linestring
	*/
	virtual bool isOnLine(const Coordinate& p, const CoordinateList* linestring) const=0;
	/**
	* Test whether a ring (simple polygon) is oriented counter-clockwise.
	*
	* @return true if the ring is oriented counter-clockwise
	*/
	virtual bool isCCW(const CoordinateList* ring) const=0;
	/**
	* Computes the orientation of a point q to the directed line segment p1-p2.
	* The orientation of a point relative to a directed line segment indicates
	* which way you turn to get to q after travelling from p1 to p2.
	*
	* @return 1 if q is counter-clockwise from p1-p2
	* @return -1 if q is clockwise from p1-p2
	* @return 0 if q is collinear with p1-p2
	*/
	virtual int computeOrientation(const Coordinate& p1, const Coordinate& p2, const Coordinate& q) const=0;
	static double distancePointLine(const Coordinate& p,const Coordinate& A,const Coordinate& B);
	static double distanceLineLine(const Coordinate& A, const Coordinate& B, const Coordinate& C, const Coordinate& D);
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
	bool isInside(const Coordinate& pt);
private:
	CGAlgorithms *cga;
	const CoordinateList* pts;
};

class LineIntersector{
public:	
	static double computeEdgeDistance(const Coordinate& p, const Coordinate& p0, const Coordinate& p1);
	static double nonRobustComputeEdgeDistance(const Coordinate& p,const Coordinate& p1,const Coordinate& p2);
	LineIntersector();
	virtual ~LineIntersector();
	virtual void setMakePrecise(const PrecisionModel *newPM);
	/**
	* Compute the intersection of a point p and the line p1-p2
	*/
	virtual void computeIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2) =0;
	enum {
		DONT_INTERSECT,
		DO_INTERSECT,
		COLLINEAR
	};
	virtual void computeIntersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& p3, const Coordinate& p4);
	virtual string toString() const;
	virtual bool hasIntersection() const;
	virtual int getIntersectionNum() const;
	virtual const Coordinate& getIntersection(int intIndex) const;
	static bool isSameSignAndNonZero(double a,double b);
	virtual bool isIntersection(const Coordinate& pt) const;
	virtual bool isProper() const;
	virtual const Coordinate& getIntersectionAlongSegment(int segmentIndex,int intIndex);
	virtual int getIndexAlongSegment(int segmentIndex,int intIndex);
	virtual double getEdgeDistance(int geomIndex,int intIndex) const;
protected:
	/**
	* If makePrecise is true, computed intersection coordinates will be made precise
	* using Coordinate#makePrecise
	*/
	const PrecisionModel *precisionModel;
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
	virtual bool isCollinear() const;
	virtual int computeIntersect(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2)=0;
	virtual bool isEndPoint() const;
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
	void computeIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2);
	int computeIntersect(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2);
private:
//	bool between(Coordinate& p1,Coordinate& p2,Coordinate& q);
	int computeCollinearIntersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2);
	Coordinate* intersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2) const;
	void normalize(Coordinate *n1,Coordinate *n2,Coordinate *n3,Coordinate *n4,Coordinate *normPt) const;
	double smallestInAbsValue(double x1,double x2,double x3,double x4) const;
};

class NonRobustLineIntersector: public LineIntersector {
public:
	NonRobustLineIntersector();
	void computeIntersection(const Coordinate& p,const Coordinate& p1,const Coordinate& p2);
protected:
	int computeIntersect(const Coordinate& p1,const Coordinate& p2,const Coordinate& p3,const Coordinate& p4);
private:
	int computeCollinearIntersection(const Coordinate& p1,const Coordinate& p2,const Coordinate& p3,const Coordinate& p4);
	double rParameter(const Coordinate& p1,const Coordinate& p2,const Coordinate& p) const;
};

class RobustCGAlgorithms: public CGAlgorithms {
public:
	static int orientationIndex(const Coordinate& p1,const Coordinate& p2,const Coordinate& q);
	RobustCGAlgorithms();
	~RobustCGAlgorithms();
	bool isCCW(const CoordinateList* ring) const;
	bool isPointInRing(const Coordinate& p, const CoordinateList* ring) const;
	bool isOnLine(const Coordinate& p,const CoordinateList* pt) const;
	int computeOrientation(const Coordinate& p1,const Coordinate& p2, const Coordinate& q) const;
private:
	bool isInEnvelope(const Coordinate& p, const CoordinateList* ring) const;
};

class NonRobustCGAlgorithms: public CGAlgorithms {
public:
	NonRobustCGAlgorithms();
	~NonRobustCGAlgorithms();
	bool isPointInRing(const Coordinate& p, const CoordinateList* ring) const;
	bool isOnLine(const Coordinate& p, const CoordinateList* pt) const;
	bool isCCW(const CoordinateList* ring) const;
	int computeOrientation(const Coordinate& p1,const Coordinate& p2,const Coordinate& q) const;
protected:
	LineIntersector *li;
};

class SimplePointInAreaLocator {
public:
	static int locate(const Coordinate& p, const Geometry *geom);
	static bool containsPointInPolygon(const Coordinate& p,const Polygon *poly);
private:
	static bool containsPoint(const Coordinate& p,const Geometry *geom);
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
	int locate(const Coordinate& p,const Geometry *geom);
	bool intersects(const Coordinate& p,const Geometry *geom);
	int locate(const Coordinate& p,const LineString *l);
	int locate(const Coordinate& p,const LinearRing *ring);
	int locate(const Coordinate& p,const Polygon *poly);
private:
	CGAlgorithms *cga;
	bool isIn;         // true if the point lies in or on any Geometry element
	int numBoundaries;    // the number of sub-elements whose boundaries the point lies in
	void computeLocation(const Coordinate& p,const Geometry *geom);
	void updateLocationInfo(int loc);
};


class MCPointInRing: public PointInRing {
public:
	MCPointInRing(LinearRing *newRing);
	virtual ~MCPointInRing();
	bool isInside(const Coordinate& pt);
	void testLineSegment(Coordinate& p,LineSegment *seg);
	class MCSelecter: public MonotoneChainSelectAction {
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
	CoordinateList *pts;
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
	Coordinate* centSum;
	double totalLength;
public:
	CentroidLine();
	virtual ~CentroidLine();
	void add(const Geometry *geom);
	void add(const CoordinateList *pts);
	Coordinate* getCentroid() const;
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
	void add(const Geometry *geom);
	void add(const CoordinateList *ring);
	Coordinate* getCentroid() const;
private:
	CGAlgorithms *cga;
	Coordinate* basePt;// the point all triangles are based at
	Coordinate* triangleCent3;// temporary variable to hold centroid of triangle
	double areasum2;        /* Partial area sum */
	Coordinate* cg3; // partial centroid sum
	void setBasePoint(const Coordinate *newbasePt);
	void add(const Polygon *poly);
	void addShell(const CoordinateList *pts);
	void addHole(const CoordinateList *pts);
	inline void addTriangle(const Coordinate &p0, const Coordinate &p1, const Coordinate &p2,bool isPositiveArea);
	static inline  void centroid3(const Coordinate &p1,const Coordinate &p2,const Coordinate &p3,Coordinate *c);
	static inline double area2(const Coordinate &p1,const Coordinate &p2,const Coordinate &p3);
};

/**
 * Computes a point in the interior of an point geometry.
 * <h2>Algorithm</h2>
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
	Coordinate* getInteriorPoint() const;
private:
	const Coordinate *centroid;
	double minDistance;
	Coordinate *interiorPoint;
	void addInterior(const Geometry *geom);
	void addInterior(const CoordinateList *pts);
	void addEndpoints(const Geometry *geom);
	void addEndpoints(const CoordinateList *pts);
	void add(const Coordinate *point);
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

class ConvexHull {
private:
	PointLocator *pointLocator;
	CGAlgorithms *cgAlgorithms;
	const Geometry *geometry;
	CoordinateList* reduce(const CoordinateList *pts);
	CoordinateList* preSort(CoordinateList *pts);
	CoordinateList* grahamScan(const CoordinateList *c);
	void radialSort(CoordinateList *p);
	int polarCompare(Coordinate o, Coordinate p, Coordinate q);
	bool isBetween(Coordinate c1, Coordinate c2, Coordinate c3);
    BigQuad* makeBigQuad(const CoordinateList *pts);
	Geometry* lineOrPolygon(CoordinateList *newCoordinates);
	CoordinateList* cleanRing(CoordinateList *original);
public:
	ConvexHull(CGAlgorithms *newCgAlgorithms);
	~ConvexHull();
	Geometry* getConvexHull(const Geometry *newGeometry);
};
}
#endif
