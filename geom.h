#include <iostream>
#include <string>
#include <vector>
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
	void setNull(void);
	static Coordinate getNull(void);
	Coordinate();
	Coordinate(double xNew, double yNew, double zNew);
	Coordinate(const Coordinate &c);
	Coordinate(double xNew, double yNew);
	virtual ~Coordinate();
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
	virtual vector<Coordinate> toVector()=0;
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
	bool isEmpty();
	void add(Coordinate c);
	int getSize();
	Coordinate getAt(int pos);
	void setAt(Coordinate c, int pos);
	void deleteAt(int pos);
	Coordinate get();
	void set(Coordinate c);
	void remove();
	vector<Coordinate> toVector();
private:
	void moveTo(int pos);
	int size;
	ListPtr head,tail,current;
};

class PrecisionModel {
public:
	static const int FIXED=1;
	static const int FLOATING=2;
	static const double maximumPreciseValue;
	static double makePrecise(double val);
	PrecisionModel(void);
	PrecisionModel(double newScale, double newOffsetX, double newOffsetY);
	PrecisionModel(const PrecisionModel &pm);
	virtual ~PrecisionModel(void);
	bool isFloating();
	double getScale();
	double getOffsetX();
	double getOffsetY();
	void toInternal(Coordinate external, Coordinate *internal);
	Coordinate toInternal(Coordinate external);
	Coordinate toExternal(Coordinate internal);
	void toExternal(Coordinate internal, Coordinate *external);
	string toString();
private:
	int modelType;
	double scale;
	double offsetX;
	double offsetY;
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
	static const int False = -1;	/// Dimension value of the empty geometry (-1).
	static const int True = -2;		/// Dimension value of non-empty geometries (= {P, L, A}).
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
	virtual ~Envelope(void);
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

class Geometry {
public:
	Geometry(void);
	Geometry(PrecisionModel precisionModel, int SRID);
	virtual ~Geometry(void);
	virtual bool isEmpty() {return 1;};

//public abstract String getGeometryType();
//public int getSRID()
//public void setSRID(int SRID)
	virtual PrecisionModel getPrecisionModel(){return PrecisionModel();};
//public abstract Coordinate[] getCoordinates()
//public abstract int getNumPoints()
//public abstract boolean isSimple()
//public boolean isValid()
//public abstract int getDimension()
//public abstract Geometry getBoundary()
//public abstract int getBoundaryDimension()
//public Geometry getEnvelope()
//public Envelope getEnvelopeInternal()
//public boolean disjoint(Geometry g)
//public boolean touches(Geometry g)
//public boolean intersects(Geometry g)
//public boolean crosses(Geometry g)
//public boolean within(Geometry g)
//public boolean contains(Geometry g)
//public boolean overlaps(Geometry g)
//public boolean relate(Geometry g, String intersectionPattern)
//public IntersectionMatrix relate(Geometry g)
//public boolean equals(Geometry g)
//public String toString()
//public String toText()
//public Geometry buffer(double distance)
//public Geometry convexHull()
//public Geometry intersection(Geometry other)
//public Geometry Union(Geometry other)
//public Geometry difference(Geometry other)
//public Geometry symDifference(Geometry other)
//public abstract boolean equalsExact(Geometry other)
//public abstract void apply(CoordinateFilter filter)
//public abstract void apply(GeometryFilter filter)
//public Object clone()
//public abstract void normalize()
//public int compareTo(Object o)
//private int getClassSortIndex()
protected:
	PrecisionModel precisionModel;
	int SRID;
	Envelope envelope;
	bool isEquivalentClass(Geometry *other);
//static bool hasNonEmptyElements(Geometry[] geometries);
	static bool hasNullElements(CoordinateList list) {return true;};
	static void reversePointOrder(CoordinateList coordinates){};
//static Coordinate minCoordinate(Coordinate[] coordinates);
//static void scroll(Coordinate[] coordinates, Coordinate firstCoordinate);
//static int indexOf(Coordinate coordinate, Coordinate[] coordinates)
//protected void checkNotGeometryCollection(Geometry g)
//protected void checkEqualSRID(Geometry other)
//protected void checkEqualPrecisionModel(Geometry other)
//protected abstract Envelope computeEnvelopeInternal()
//protected abstract int compareToSameClass(Object o)
	int compare(vector<Coordinate> a, vector<Coordinate> b){return 0;}; //Possibly add another vector type
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
	virtual void filter(Geometry *geom)=0;
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
	virtual ~LineSegment(void);
	virtual void setCoordinates(Coordinate c0, Coordinate c1);
};

class GeometryCollectionIterator {
public:

};
class GeometryFactory {
public:

};

class IntersectionMatrix {
public:
	IntersectionMatrix();
	IntersectionMatrix(string elements);
	IntersectionMatrix(const IntersectionMatrix &im);
	virtual ~IntersectionMatrix();
	static bool matches(int actualDimensionValue, char requiredDimensionSymbol);
	static bool matches(string actualDimensionSymbols, string requiredDimensionSymbols);
	void set(int row, int column, int dimensionValue);
	void set(string dimensionSymbols);
	void setAtLeast(int row, int column, int minimumDimensionValue);
	void setAtLeastIfValid(int row, int column, int minimumDimensionValue);
	void setAtLeast(string minimumDimensionSymbols);
	void setAll(int dimensionValue);
	int get(int row, int column);
	bool isDisjoint();
	bool isIntersects();
	bool isTouches(int dimensionOfGeometryA, int dimensionOfGeometryB);
	bool isCrosses(int dimensionOfGeometryA, int dimensionOfGeometryB);
	bool isWithin();
	bool isContains();
	bool isEquals(int dimensionOfGeometryA, int dimensionOfGeometryB);
	bool isOverlaps(int dimensionOfGeometryA, int dimensionOfGeometryB);
	bool matches(string requiredDimensionSymbols);
	IntersectionMatrix *transpose();
	string toString();
private:
	int matrix[3][3];
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
bool operator==(PrecisionModel a, PrecisionModel b);

class SFSGeometry {
public:
//	virtual int getSRID()=0;
//	virtual string getGeometryType()=0;
//	virtual PrecisionModel getPrecisionModel()=0;
//	virtual Envelope getEnvelopeInternal()=0;
//	virtual Geometry getEnvelope()=0;
//	virtual bool isEmpty()=0;
//	virtual bool isSimple()=0;
//	virtual Geometry getBoundary()=0;
//	virtual int getDimension()=0;
//	virtual bool equals(Geometry other)=0;
//	virtual bool disjoint(Geometry other)=0;
//	virtual bool intersects(Geometry other)=0;
//	virtual bool touches(Geometry other)=0;
//	virtual bool crosses(Geometry other)=0;
//	virtual bool within(Geometry other)=0;
//	virtual bool contains(Geometry other)=0;
//	virtual bool overlaps(Geometry other)=0;
//	virtual bool relate(Geometry other, string intersectionPattern)=0;
//	virtual IntersectionMatrix relate(Geometry other)=0;
//	virtual Geometry buffer(double distance)=0;
//	virtual Geometry convexHull()=0;
//	virtual Geometry intersection(Geometry other)=0;
//	virtual Geometry Union(Geometry other)=0;
//	virtual Geometry difference(Geometry other)=0;
//	virtual Geometry symDifference(Geometry other)=0;
//	virtual string toText()=0;
};

class SFSGeometryCollection : public SFSGeometry {
public:
//	virtual int getNumGeometries()=0;
//	virtual Geometry getGeometryN(int n)=0;
};

class GeometryCollection : public Geometry, public SFSGeometryCollection {
public:
	GeometryCollection(void);
	GeometryCollection(Geometry *geometry,PrecisionModel pm, int b);
	virtual ~GeometryCollection(void);
};

class SFSMultiCurve : public SFSGeometryCollection {
public:
	virtual bool isClosed()=0;
};

class SFSMultiLineString : public SFSMultiCurve {
};

class SFSMultiPoint : public SFSGeometryCollection {
};

class MultiPoint: public GeometryCollection, public SFSMultiPoint {
public:
	MultiPoint(){};
	MultiPoint(Geometry *geometry,PrecisionModel pm, int b){};
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

class Point : public Geometry, public SFSPoint {
public:
	Point(void);
	Point(Coordinate c, PrecisionModel precisionModel, int SRID);
	Point(const Point &p); //replaces clone()
	virtual ~Point(void);
	CoordinateList getCoordinates(void);
	int getNumPoints();
	bool isEmpty();
	bool isSimple();
	bool isValid();
	int getDimension();
	int getBoundaryDimension();
	double getX();
	double getY();
	Coordinate getCoordinate();
	string getGeometryType();
	Geometry getBoundary();
	void apply(CoordinateFilter *filter);
	void apply(GeometryFilter *filter);
	bool equalsExact(Geometry *other);
	void normalize(void) { };
protected:
	Coordinate coordinate;
	Envelope computeEnvelopeInternal();
	int compareToSameClass(Point p);
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

class LineString: public Geometry, public SFSLineString {
public:
	LineString();
	LineString(const LineString &ls);
	LineString(CoordinateList newPoints, PrecisionModel precisionModel, int SRID);
	virtual ~LineString();
	virtual CoordinateList getCoordinates();
	virtual Coordinate getCoordinateN(int n);
	virtual int getDimension();
	virtual int getBoundaryDimension();
	virtual bool isEmpty();
	virtual int getNumPoints();
	virtual Point getPointN(int n);
	virtual Point getStartPoint();
	virtual Point getEndPoint();
	virtual bool isClosed();
	virtual bool isRing();
	virtual string getGeometryType();
	virtual bool isSimple();
	virtual Geometry getBoundary();
	virtual bool isCoordinate(Coordinate pt);
	virtual bool equalsExact(Geometry *other);
	virtual void apply(CoordinateFilter *filter);
	virtual void apply(GeometryFilter *filter);
	virtual void normalize();
protected:
	CoordinateList points;
	virtual Envelope computeEnvelopeInternal();
	virtual int compareToSameClass(LineString ls);
	virtual bool isEquivalentClass(Geometry *other);
};

class LinearRing : public LineString, public SFSLinearRing {
public:
	LinearRing();
	LinearRing(const LinearRing &lr);
	LinearRing(CoordinateList points,PrecisionModel precisionModel,int SRID);
	virtual ~LinearRing();
	bool isSimple();
	string getGeometryType();
	bool isClosed();
};

class SFSSurface : public SFSGeometry {
};

class SFSPolygon : public SFSSurface {
public:
	virtual LineString getExteriorRing()=0;
	virtual int getNumInteriorRing()=0;
	virtual LineString getInteriorRingN(int n)=0;
};