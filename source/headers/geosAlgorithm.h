#ifndef GEOS_ALGORITHM_H
#define GEOS_ALGORITHM_H

#include "geom.h"
#include "platform.h"

class Coordinate;

class NotRepresentableException {
public:
	NotRepresentableException();
	NotRepresentableException(string msg);
	~NotRepresentableException();
	string toString();
private:
	string txt;
};

class PointInRing{
public:
//	PointInRing();
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
	static Coordinate& intersection(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2);
	double x,y,w;
	HCoordinate();
	HCoordinate(double _x, double _y, double _w);
	HCoordinate(Coordinate& p);
	HCoordinate(HCoordinate p1, HCoordinate p2);
	double getX();
	double getY();
	Coordinate& getCoordinate();
};

class SimplePointInRing: public PointInRing {
public:
	SimplePointInRing(LinearRing *ring);
	bool isInside(Coordinate& pt);
private:
	static CGAlgorithms *cga;
	CoordinateList* pts;
};

class LineIntersector{
public:	
	static double computeEdgeDistance(Coordinate& p,Coordinate& p0,Coordinate& p1);
	static double nonRobustComputeEdgeDistance(Coordinate& p,Coordinate& p1,Coordinate& p2);
	LineIntersector();
	virtual void setMakePrecise(bool newMakePrecise);
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
	bool makePrecise;
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
	void computeIntersection(Coordinate& p,Coordinate& p1,Coordinate& p2);
	int computeIntersect(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2);
private:
	bool between(Coordinate& p1,Coordinate& p2,Coordinate& q);
	int computeCollinearIntersection(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2);
	Coordinate& intersection(Coordinate& p1,Coordinate& p2,Coordinate& q1,Coordinate& q2);
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
	RobustLineIntersector* lineIntersector;
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
private:
	static CGAlgorithms *cga;
	static bool containsPoint(Coordinate& p,Geometry *geom);
	static bool containsPointInPolygon(Coordinate& p,Polygon *poly);
};

class PointLocator {
public:
	PointLocator();
	~PointLocator();
	int locate(Coordinate& p,Geometry *geom);
protected:
	CGAlgorithms *cga;
	bool isIn;         // true if the point lies in or on any Geometry element
	int numBoundaries;    // the number of sub-elements whose boundaries the point lies in
private:
	void computeLocation(Coordinate& p,Geometry *geom);
	void updateLocationInfo(int loc);
	int locate(Coordinate& p,LineString *l);
	int locate(Coordinate& p,LinearRing *ring);
	int locate(Coordinate& p,Polygon *poly);
};


class MCPointInRing: public PointInRing {
public:
	MCPointInRing(LinearRing *newRing);
	bool isInside(Coordinate& pt);

  //class MCSelecter extends MonotoneChainSelectAction
  //{
  //  Coordinate p;

  //  public MCSelecter(Coordinate p)
  //  {
  //    this.p = p;
  //  }

  //  public void select(LineSegment ls)
  //  {
  //    testLineSegment(p, ls);
  //  }
  //}

//private:
//	LinearRing *ring;
//	IntervalTree *intTree;
//	int crossings = 0;  // number of segment/ray crossings
  //private void buildIndex()
  //{
  //  Envelope env = ring.getEnvelopeInternal();
  //  intTree = new IntervalTree(env.getMinY(), env.getMaxY());

  //  Coordinate[] pts = ring.getCoordinates();
  //  List mcList = MonotoneChainBuilder.getChains(pts);

  //  for (int i = 0; i < mcList.size(); i++) {
  //    MonotoneChain mc = (MonotoneChain) mcList.get(i);
  //    Envelope mcEnv = mc.getEnvelope();
  //    intTree.insert(mcEnv.getMinY(), mcEnv.getMaxY(), mc);
  //  }
  //}
  //private void testMonotoneChain(Envelope rayEnv, MCSelecter mcSelecter, MonotoneChain mc)
  //{
  //  mc.select(rayEnv, mcSelecter);
  //}

  //private void testLineSegment(Coordinate p, LineSegment seg) {
  //  double xInt;  // x intersection of segment with ray
  //  double x1;    // translated coordinates
  //  double y1;
  //  double x2;
  //  double y2;

  //  /*
  //   *  Test if segment crosses ray from test point in positive x direction.
  //   */
  //  Coordinate p1 = seg.p0;
  //  Coordinate p2 = seg.p1;
  //  x1 = p1.x - p.x;
  //  y1 = p1.y - p.y;
  //  x2 = p2.x - p.x;
  //  y2 = p2.y - p.y;

  //  if (((y1 > 0) && (y2 <= 0)) ||
  //      ((y2 > 0) && (y1 <= 0))) {
  //      /*
  //       *  segment straddles x axis, so compute intersection.
  //       */
  //    xInt = RobustDeterminant.signOfDet2x2(x1, y1, x2, y2) / (y2 - y1);
  //      //xsave = xInt;
  //      /*
  //       *  crosses ray if strictly positive intersection.
  //       */
  //    if (0.0 < xInt) {
  //      crossings++;
  //    }
  //  }
  //}

};

#endif
