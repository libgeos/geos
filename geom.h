#include <iostream>
#include <string>
#include "math.h"
#include "platform.h"

using namespace std;

/**
 *  <code>Coordinate</code> is the lightweight class used to store coordinates.
 *  It is distinct from <code>Point</code>, which is a subclass of <code>Geometry</code>
 *  . Unlike objects of type <code>Point</code> (which contain additional
 *  information such as an envelope, a precision model, and spatial reference
 *  system information), a <code>Coordinate</code> only contains ordinate values
 *  and accessor methods. <P>
 *
 *  <code>Coordinate</code>s are two-dimensional points, with an additional
 *  z-ordinate. JTS does not support any operations on the z-ordinate except
 *  the basic accessor functions. Constructed coordinates will have a
 *  z-ordinate of <code>DoubleNotANumber</code>.  The standard comparison functions will ignore
 *  the z-ordinate.
 *
 */
 class Coordinate {
public:
	Coordinate();
	Coordinate(double xNew, double yNew, double zNew);
	Coordinate(const Coordinate &c);
	Coordinate(double xNew, double yNew);
	~Coordinate();
	void setCoordinate(Coordinate other);
	bool equals2D(Coordinate other);
	int compareTo(Coordinate other);
	bool equals3D(Coordinate other);
	string toString();
	void makePrecise();
	double distance(Coordinate p);

	double x;	/// x-coordinate
	double y;	/// y-coordinate
	double z;	/// z-coordinate
};

/**
 *  <code>Geometry</code> classes support the concept of applying a
 *  coordinate filter to every coordinate in the <code>Geometry</code>. A
 *  coordinate filter can either record information about each coordinate or
 *  change the coordinate in some way. Coordinate filters implement the
 *  interface <code>CoordinateFilter</code>. (<code>CoordinateFilter</code> is
 *  an example of the Gang-of-Four Visitor pattern). Coordinate filters can be
 *  used to implement such things as coordinate transformations, centroid and
 *  envelope computation, and many other functions.
 *
 */
class CoordinateFilter {
public:
  /**
   *  Performs an operation with or on <code>coord</code>.
   *
   *@param  coord  a <code>Coordinate</code> to which the filter is applied.
   */
   virtual void filter(Coordinate coord)=0;
};

/**
 *  Constants representing the dimensions of a point, a curve and a surface.
 *  Also, constants representing the empty geometry, non-empty geometries and
 *  any geometry.
 *
 */
class Dimension {
public:
	static const int P = 0;			/// Dimension value of a point (0).
	static const int L = 1;			/// Dimension value of a curve (1).
	static const int A = 2;			/// Dimension value of a surface (2).
	static const int FALSE = -1;	/// Dimension value of the empty geometry (-1).
	static const int TRUE = -2;		/// Dimension value of non-empty geometries (= {P, L, A}).
	static const int DONTCARE = -3;	/// Dimension value for any dimension (= {FALSE, TRUE}).
	static char toDimensionSymbol(int dimensionValue);
	static int toDimensionValue(char dimensionSymbol);
};

/**
 *  An Envelope defines a rectangulare region of the 2D coordinate plane.
 *  It is often used to represent the bounding box of a Geometry,
 *  e.g. the minimum and maximum x and y values of the Coordinates.
 *  <p>
 *  Note that Envelopes support infinite or half-infinite regions, by using the values of
 *  <code>Double_POSITIVE_INFINITY</code> and <code>Double_NEGATIVE_INFINITY</code>.
 *  <p>
 *  When Envelope objects are created or initialized,
 *  the supplies extent values are automatically sorted into the correct order.
 *
 */
class Envelope {
public:
	Envelope(void);
	Envelope(double x1, double x2, double y1, double y2);
	Envelope(Coordinate p1, Coordinate p2);
	Envelope(Coordinate p);
	Envelope(const Envelope &env);
	~Envelope(void);
	void init(void);
	void init(double x1, double x2, double y1, double y2);
	void init(Coordinate p1, Coordinate p2);
	void init(Coordinate p);
	void init(Envelope env);
	void setToNull(void);
	bool isNull(void);
	double getWidth(void);
	double getHeight(void);
	double getMaxY();
	double getMaxX();
	double getMinY();
	double getMinX();
	void expandToInclude(Coordinate p);
	void expandToInclude(double x, double y);
	void expandToInclude(Envelope other);
	bool contains(Coordinate p);
	bool contains(double x, double y);
	bool contains(Envelope other);
	bool overlaps(Coordinate p);
	bool overlaps(double x, double y);
	bool overlaps(Envelope other);
	string toString(void);

private:
	double minx;	/// the minimum x-coordinate
	double maxx;	/// the maximum x-coordinate
	double miny;	/// the minimum y-coordinate
	double maxy;	/// the maximum y-coordinate
};

class PrecisionModel {
public:
	PrecisionModel(void);
	~PrecisionModel(void);
};

class Geometry {
public:
	Geometry(void);
	Geometry(PrecisionModel precisionModel, int SRID);
	~Geometry(void);
protected:
	PrecisionModel precisionModel;
	int SRID;
};

/**
 *  <code>Geometry</code> classes support the concept of applying a <code>Geometry</code>
 *  filter to the <code>Geometry</code>. In the case of <code>GeometryCollection</code>
 *  subclasses, the filter is applied to every element <code>Geometry</code>. A
 *  <code>Geometry</code> filter can either record information about the <code>Geometry</code>
 *  or change the <code>Geometry</code> in some way. <code>Geometry</code>
 *  filters implement the interface <code>GeometryFilter</code>. (<code>GeometryFilter</code>
 *  is an example of the Gang-of-Four Visitor pattern).
 *
 */
class GeometryFilter {
public:
  /**
   *  Performs an operation with or on <code>geom</code>.
   *
   *@param  geom  a <code>Geometry</code> to which the filter is applied.
   */
	virtual void filter(Geometry geom)=0;
};

/**
 *  Basic implementation of <code>LineSegment</code>.
 *
 */
class LineSegment {
public:
	Coordinate p0; /// Segment start
	Coordinate p1; /// Segemnt end
	LineSegment(void);
	LineSegment(Coordinate c0, Coordinate c1);
	~LineSegment(void);
	void setCoordinates(Coordinate c0, Coordinate c1);
};

class GeometryCollection {
public:

};
class GeometryCollectionIterator {
public:

};
class GeometryFactory {
public:

};
class IntersectionMatrix {
public:

};
class LinearRing {
public:

};
class LineString {
public:

};

/**
 *  Constants representing the location of a point relative to a geometry. They
 *  can also be thought of as the row or column index of a DE-9IM matrix. For a
 *  description of the DE-9IM, see the <A
 *  HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
 *  Specification for SQL</A> .
 *
 */
class Location {
public:
  /**
   *  DE-9IM row index of the interior of the first geometry and column index of
   *  the interior of the second geometry. Location value for the interior of a
   *  geometry.
   */
	static const int INTERIOR = 0;
  /**
   *  DE-9IM row index of the boundary of the first geometry and column index of
   *  the boundary of the second geometry. Location value for the boundary of a
   *  geometry.
   */
	static const int BOUNDARY = 1;
  /**
   *  DE-9IM row index of the exterior of the first geometry and column index of
   *  the exterior of the second geometry. Location value for the exterior of a
   *  geometry.
   */
	static const int EXTERIOR = 2;
  /**
   *  Used for uninitialized location values.
   */
	static const int UNDEF = -1;   ///Instead of NULL
	static char toLocationSymbol(int locationValue);
};

class MultiLineString {
public:

};
class MultiPoint {
public:

};
class MultiPolygon {
public:

};

class Polygon {
public:

};

class TopologyException {
public:

};

//Operators
bool operator==(Coordinate a, Coordinate b);
bool operator==(Envelope a, Envelope b);

class SFSGeometry {
public:
	virtual int getSRID()=0;
	virtual string getGeometryType()=0;
	virtual PrecisionModel getPrecisionModel()=0;
	virtual Envelope getEnvelopeInternal()=0;
	virtual Geometry getEnvelope()=0;
	virtual bool isEmpty()=0;
	virtual bool isSimple()=0;
	virtual Geometry getBoundary()=0;
	virtual int getDimension()=0;
	virtual bool equals(Geometry other)=0;
	virtual bool disjoint(Geometry other)=0;
	virtual bool intersects(Geometry other)=0;
	virtual bool touches(Geometry other)=0;
	virtual bool crosses(Geometry other)=0;
	virtual bool within(Geometry other)=0;
	virtual bool contains(Geometry other)=0;
	virtual bool overlaps(Geometry other)=0;
	virtual bool relate(Geometry other, string intersectionPattern)=0;
	virtual IntersectionMatrix relate(Geometry other)=0;
	virtual Geometry buffer(double distance)=0;
	virtual Geometry convexHull()=0;
	virtual Geometry intersection(Geometry other)=0;
	virtual Geometry Union(Geometry other)=0;
	virtual Geometry difference(Geometry other)=0;
	virtual Geometry symDifference(Geometry other)=0;
	virtual string toText()=0;
};

class SFSGeometryCollection : public SFSGeometry {
public:
	virtual int getNumGeometries()=0;
	virtual Geometry getGeometryN(int n)=0;
};

class SFSMultiCurve : public SFSGeometryCollection {
public:
	virtual bool isClosed()=0;
};

class SFSMultiLineString : public SFSMultiCurve {
};

class SFSMultiPoint : public SFSGeometryCollection {
};

class SFSMultiSurface : public SFSGeometryCollection {
};

class SFSMultiPolygon : public SFSMultiSurface {
};

class SFSPoint : public SFSGeometry {
public:
	virtual double getX()=0;
	virtual double getY()=0;
	virtual Coordinate getCoordinate()=0;
};

class Point : public Geometry { //, public SFSPoint {
public:
	Point(void);
	Point(Coordinate c, PrecisionModel precisionModel, int SRID);
	~Point(void);
protected:
	Coordinate coordinate;
};

class  SFSCurve : public SFSGeometry {
public:
	virtual Point getStartPoint()=0;
	virtual Point getEndPoint()=0;
	virtual bool isClosed()=0;
	virtual bool isRing()=0;
	virtual bool isSimple()=0;
};

class SFSLineString : public SFSCurve  {
public:
	virtual int getNumPoints()=0;
	virtual Point getPointN(int n)=0;
	virtual Coordinate getCoordinateN(int n)=0;

};

class SFSLinearRing : public SFSLineString {
};

class SFSSurface : public SFSGeometry {
};

class SFSPolygon : public SFSSurface {
public:
	virtual LineString getExteriorRing()=0;
	virtual int getNumInteriorRing()=0;
	virtual LineString getInteriorRingN(int n)=0;
};

class CoordinateListInterface {
public:
	virtual void reset()=0;
	virtual Coordinate getNext()=0;
	virtual bool hasNext()=0;
	virtual void add(Coordinate c)=0;
	virtual int getSize()=0;
	virtual Coordinate getAt(int pos)=0;
	virtual void setAt(Coordinate c, int pos)=0;
	virtual void deleteAt(int pos)=0;
	virtual Coordinate get()=0;
	virtual void set(Coordinate c)=0;
	virtual void remove()=0;
};

class CoordinateList : public CoordinateListInterface {
	typedef struct List {
		Coordinate data;
		List* next;
		List* prev;
		List(){
			next=NULL;
			prev=NULL;
			Coordinate data;
		}
		List(Coordinate c){
			next=NULL;
			prev=NULL;
			data=Coordinate(c);
		}
	};
	typedef List* ListPtr;
public:
	CoordinateList();
	CoordinateList(Coordinate c);
	~CoordinateList();
	void reset();
	Coordinate getNext();
	bool hasNext();
	void add(Coordinate c);
	int getSize();
	Coordinate getAt(int pos);
	void setAt(Coordinate c, int pos);
	void deleteAt(int pos);
	Coordinate get();
	void set(Coordinate c);
	void remove();
private:
	void moveTo(int pos);
	int size;
	ListPtr head,tail,current;
};