#ifndef GEOS_GEOM_H
#define GEOS_GEOM_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
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
//	virtual ~Coordinate();
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

class CoordinateList {
public:
	virtual	bool isEmpty()=0;
	virtual	void add(Coordinate c)=0;
	virtual	int getSize()=0;
	virtual	Coordinate& getAt(int pos)=0;
	virtual	void setAt(Coordinate c, int pos)=0;
	virtual	void deleteAt(int pos)=0;
	virtual	vector<Coordinate>* toVector()=0;
	virtual	string toString()=0;
	virtual	void setPoints(const vector<Coordinate> &v)=0;
	bool hasRepeatedPoints();
	Coordinate* minCoordinate();
};

class BasicCoordinateList : public CoordinateList {
public:
	BasicCoordinateList();
	BasicCoordinateList(int n);
	BasicCoordinateList(Coordinate c);
	BasicCoordinateList(const BasicCoordinateList &cl);
	~BasicCoordinateList();
	bool isEmpty();
	void add(Coordinate c);
	int getSize();
	Coordinate& getAt(int pos);
	void setAt(Coordinate c, int pos);
	void deleteAt(int pos);
	vector<Coordinate>* toVector();
	string toString();
	void setPoints(const vector<Coordinate> &v);
private:
	vector<Coordinate> *vect;
};

struct point_3d {
	double x;
	double y;
	double z;
};

class PointCoordinateList : public CoordinateList {
public:
	PointCoordinateList();
	PointCoordinateList(int n);
	PointCoordinateList(Coordinate c);
	PointCoordinateList(const PointCoordinateList &cl);
	~PointCoordinateList();
	bool isEmpty();
	void add(Coordinate c);
	int getSize();
	Coordinate& getAt(int pos);
	void setAt(Coordinate c, int pos);
	void deleteAt(int pos);
	vector<Coordinate>* toVector();
	string toString();
	void setPoints(const vector<Coordinate> &v);
private:
	vector<point_3d> *vect;
};

class CoordinateListFactory {
public:
	virtual CoordinateList* createCoordinateList()=0;
	virtual CoordinateList* createCoordinateList(int size)=0;
	virtual CoordinateList* createCoordinateList(Coordinate c)=0;
	static CoordinateListFactory* internalFactory;
};

class BasicCoordinateListFactory: public CoordinateListFactory {
	CoordinateList* createCoordinateList() {return new BasicCoordinateList();};
	CoordinateList* createCoordinateList(int size) {return new BasicCoordinateList(size);};
	CoordinateList* createCoordinateList(Coordinate c) {return new BasicCoordinateList(c);};
};

class PointCoordinateListFactory: public CoordinateListFactory {
	CoordinateList* createCoordinateList() {return new PointCoordinateList();};
	CoordinateList* createCoordinateList(int size) {return new PointCoordinateList(size);};
	CoordinateList* createCoordinateList(Coordinate c) {return new PointCoordinateList(c);};
};

class PrecisionModel {
public:
	enum {
		FIXED=1,
		FLOATING
	};
//	static const int FIXED=1;
//	static const int FLOATING=2;
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
	void toInternal(Coordinate& external, Coordinate *internal);
	Coordinate* toInternal(Coordinate& external);
	Coordinate* toExternal(Coordinate& internal);
	void toExternal(Coordinate& internal, Coordinate *external);
	string toString();
	void round(Coordinate& p0,Coordinate& p1);
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
 *  <code>Geometry</code> classes support the concept of applying
 *  a <code>GeometryComponentFilter</code>
 *  filter to the <code>Geometry</code>.
 *  The filter is applied to every component of the <code>Geometry</code>
 *  which is itself a <code>Geometry</code>.
 *  A <code>GeometryComponentFilter</code> filter can either
 *  record information about the <code>Geometry</code>
 *  or change the <code>Geometry</code> in some way.
 *  <code>GeometryComponentFilter</code>
 *  is an example of the Gang-of-Four Visitor pattern.
 *
 */
class Geometry;
class GeometryComponentFilter {
public:
	/**
	*  Performs an operation with or on <code>geom</code>.
	*
	*@param  geom  a <code>Geometry</code> to which the filter is applied.
	*/
	virtual void filter(Geometry *geom)=0;
};


/**
 *  Constants representing the dimensions of a point, a curve and a surface.
 *  Also, constants representing the empty geometry, non-empty geometries and
 *  any geometry.
 *
 */
class Dimension {
public:
	enum {
		DONTCARE=-3,	/// Dimension value for any dimension (= {FALSE, TRUE}).
		True,			/// Dimension value of non-empty geometries (= {P, L, A}).
		False,			/// Dimension value of the empty geometry (-1).
		P,				/// Dimension value of a point (0).
		L,				/// Dimension value of a curve (1).
		A				/// Dimension value of a surface (2).
	};
	//static const int P = 0;			/// Dimension value of a point (0).
	//static const int L = 1;			/// Dimension value of a curve (1).
	//static const int A = 2;			/// Dimension value of a surface (2).
	//static const int False = -1;	/// Dimension value of the empty geometry (-1).
	//static const int True = -2;		/// Dimension value of non-empty geometries (= {P, L, A}).
	//static const int DONTCARE = -3;	/// Dimension value for any dimension (= {FALSE, TRUE}).
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
	Envelope(Coordinate& p1, Coordinate& p2);
	Envelope(Coordinate& p);
	Envelope(const Envelope &env);
	virtual ~Envelope(void);
	void init(void);
	void init(double x1, double x2, double y1, double y2);
	void init(Coordinate& p1, Coordinate& p2);
	void init(Coordinate& p);
	void init(Envelope env);
	void setToNull(void);
	bool isNull(void);
	double getWidth(void);
	double getHeight(void);
	double getMaxY();
	double getMaxX();
	double getMinY();
	double getMinX();
	void expandToInclude(Coordinate& p);
	void expandToInclude(double x, double y);
	void expandToInclude(Envelope other);
	bool contains(Coordinate& p);
	bool contains(double x, double y);
	bool contains(Envelope other);
	bool overlaps(Coordinate& p);
	bool overlaps(double x, double y);
	bool overlaps(Envelope other);
	string toString(void);
	double distance(Envelope env);
private:
	static double distance(double x0,double y0,double x1,double y1);
	double minx;	/// the minimum x-coordinate
	double maxx;	/// the maximum x-coordinate
	double miny;	/// the minimum y-coordinate
	double maxy;	/// the maximum y-coordinate
};

class Geometry;
class GeometryFilter;
class IntersectionMatrix;

class SFSGeometry {
public:
	virtual int getSRID()=0;
	virtual string getGeometryType()=0;
	virtual PrecisionModel getPrecisionModel()=0;
	virtual Envelope getEnvelopeInternal()=0;
	virtual Geometry getEnvelope()=0;
	virtual bool isEmpty()=0;
	virtual bool isSimple()=0;
	virtual Geometry* getBoundary()=0;
	virtual int getDimension()=0;
	virtual bool equals(Geometry *other)=0;
	virtual bool disjoint(Geometry *other)=0;
	virtual bool intersects(Geometry *other)=0;
	virtual bool touches(Geometry *other)=0;
	virtual bool crosses(Geometry *other)=0;
	virtual bool within(Geometry *other)=0;
	virtual bool contains(Geometry *other)=0;
	virtual bool overlaps(Geometry *other)=0;
	virtual bool relate(Geometry *other, string intersectionPattern)=0;
	virtual IntersectionMatrix relate(Geometry *other)=0;
	virtual Geometry buffer(double distance)=0;
	virtual Geometry convexHull()=0;
	virtual Geometry intersection(Geometry *other)=0;
	virtual Geometry Union(Geometry *other)=0;
	virtual Geometry difference(Geometry *other)=0;
	virtual Geometry symDifference(Geometry *other)=0;
	virtual string toText()=0;
};

class CGAlgorithms;
class Geometry: public SFSGeometry {
public:
	Geometry(void);
	Geometry(const Geometry &geom);
	Geometry(PrecisionModel precisionModel, int SRID);
	virtual string getGeometryType(){return "Geometry";}; //Abstract
	virtual int getSRID();
	virtual void setSRID(int newSRID);
	virtual PrecisionModel getPrecisionModel();
	virtual Coordinate* getCoordinate(){return new Coordinate();}; //Abstract
	virtual CoordinateList* getCoordinates(){return new BasicCoordinateList();}; //Abstract
	virtual int getNumPoints(){return 0;}; //Abstract
	virtual bool isSimple() {return false;}; //Abstract
	virtual bool isValid();
	virtual bool isEmpty() {return false;}; //Abstract
	virtual int getDimension(){return 0;}; //Abstract
	virtual Geometry* getBoundary() {return new Geometry();}; //Abstract
	virtual int getBoundaryDimension(){return 0;}; //Abstract
	virtual Geometry getEnvelope();
	virtual Envelope getEnvelopeInternal();
	virtual bool disjoint(Geometry *g);
	virtual bool touches(Geometry *g);
	virtual bool intersects(Geometry *g);
	virtual bool crosses(Geometry *g);
	virtual bool within(Geometry *g);
	virtual bool contains(Geometry *g);
	virtual bool overlaps(Geometry *g);
	virtual bool relate(Geometry *g, string intersectionPattern);
	virtual IntersectionMatrix relate(Geometry *g);
	virtual bool equals(Geometry *g);
	virtual string toString();
	virtual string toText();
	virtual Geometry buffer(double distance);
	virtual Geometry convexHull();
	virtual Geometry intersection(Geometry *other);
	virtual Geometry Union(Geometry *other);
	virtual Geometry difference(Geometry *other);
	virtual Geometry symDifference(Geometry *other);
	virtual bool equalsExact(Geometry *other){return false;}; //Abstract
	virtual void apply(CoordinateFilter *filter){}; //Abstract
	virtual void apply(GeometryFilter *filter){}; //Abstract
	virtual void apply(GeometryComponentFilter *filter){};
	//public Object clone() // Replaced by copy constructor
	virtual void normalize(){}; //Abstract
	virtual int compareTo(Geometry *geom);
	virtual double distance(Geometry *g);
	virtual double getArea();
	virtual double getLength();
	virtual ~Geometry(void);
protected:
	PrecisionModel precisionModel;
	int SRID;
	Envelope envelope;
	static CGAlgorithms *cgAlgorithms;
	static bool hasNonEmptyElements(vector<Geometry *> geometries);
	static bool hasNullElements(CoordinateList* list);
	static bool hasNullElements(vector<Geometry *> lrs);
	static void reversePointOrder(CoordinateList* coordinates);
//	static Coordinate& minCoordinate(CoordinateList* coordinates);
	static void scroll(CoordinateList* coordinates,Coordinate* firstCoordinate);
	static int indexOf(Coordinate* coordinate,CoordinateList* coordinates);
	virtual bool isEquivalentClass(Geometry *other);
	virtual void checkNotGeometryCollection(Geometry *g);
	virtual void checkEqualSRID(Geometry *other);
	virtual void checkEqualPrecisionModel(Geometry *other);
	virtual Envelope* computeEnvelopeInternal(){return new Envelope();}; //Abstract
	virtual int compareToSameClass(Geometry *geom){return 0;}; //Abstract
	int compare(vector<Coordinate> a, vector<Coordinate> b);
	int compare(vector<Geometry *> a, vector<Geometry *> b);
private:
	vector<string> sortedClasses;
	virtual int getClassSortIndex();

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
	LineSegment(const LineSegment &ls);
	LineSegment(Coordinate c0, Coordinate c1);
	virtual ~LineSegment(void);
	virtual void setCoordinates(Coordinate c0, Coordinate c1);
	virtual Coordinate& getCoordinate(int i);
	virtual void setCoordinates(LineSegment ls);
	virtual void reverse();
	virtual void normalize();
	virtual double angle();
	virtual double distance(LineSegment ls);
	virtual double distance(Coordinate& p);
	virtual double projectionFactor(Coordinate& p);
	virtual Coordinate project(Coordinate& p);
	virtual int compareTo(LineSegment other);
	virtual bool equalsTopo(LineSegment other);
	virtual string toString();
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
	IntersectionMatrix* transpose();
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
	enum {
		/**
		*  Used for uninitialized location values.
		*/
		UNDEF=-1,   ///Instead of NULL
		/**
		*  DE-9IM row index of the interior of the first geometry and column index of
		*  the interior of the second geometry. Location value for the interior of a
		*  geometry.
		*/
		INTERIOR,
		/**
		*  DE-9IM row index of the boundary of the first geometry and column index of
		*  the boundary of the second geometry. Location value for the boundary of a
		*  geometry.
		*/
		BOUNDARY,
		/**
		*  DE-9IM row index of the exterior of the first geometry and column index of
		*  the exterior of the second geometry. Location value for the exterior of a
		*  geometry.
		*/
		EXTERIOR = 2
	};
	//static const int INTERIOR = 0;
	//static const int BOUNDARY = 1;
	//static const int EXTERIOR = 2;
	//static const int UNDEF = -1;   ///Instead of NULL
	static char toLocationSymbol(int locationValue);
};

class TopologyException {
public:

};

//Operators
bool operator==(Coordinate a, Coordinate b);
bool operator==(Envelope a, Envelope b);
bool operator==(PrecisionModel a, PrecisionModel b);
bool operator==(LineSegment a, LineSegment b);

bool lessThen(Coordinate& a,Coordinate& b);
bool greaterThen(Geometry *first, Geometry *second);

class SFSGeometryCollection { //: public SFSGeometry {
public:
	virtual int getNumGeometries()=0;
	virtual Geometry* getGeometryN(int n)=0;
};

class GeometryCollection : public Geometry, public SFSGeometryCollection {
public:
	GeometryCollection(void);
	GeometryCollection(const GeometryCollection &gc);
	GeometryCollection(vector<Geometry *> *newGeometries,PrecisionModel* pm, int SRID);
	virtual ~GeometryCollection(void);
	virtual CoordinateList* getCoordinates();
	virtual bool isEmpty();
	virtual int getDimension();
	virtual int getBoundaryDimension();
	virtual int getNumGeometries();
	virtual Geometry* getGeometryN(int n);
	virtual int getNumPoints();
	virtual string getGeometryType();
	virtual bool isSimple();
	virtual Geometry* getBoundary();
	virtual bool equalsExact(Geometry *other);
	virtual void apply(CoordinateFilter *filter);
	virtual void apply(GeometryFilter *filter);
	virtual void apply(GeometryComponentFilter *filter);
	virtual void normalize();
	virtual Coordinate* getCoordinate();
	virtual double getArea();
	virtual double getLength();
protected:
	vector<Geometry *>* geometries;
	virtual Envelope* computeEnvelopeInternal();
	virtual int compareToSameClass(GeometryCollection *gc);
};

class GeometryCollectionIterator {
public:
	GeometryCollectionIterator();
	GeometryCollectionIterator(const GeometryCollectionIterator &gci);
	GeometryCollectionIterator(GeometryCollection *newParent);
	virtual ~GeometryCollectionIterator();
	bool hasNext();
	Geometry *next();
	void remove(); //Not implemented
private:
	GeometryCollection* parent;
	bool atStart;
	int max;
	int index;
	GeometryCollectionIterator* subcollectionIterator;
};

class SFSMultiCurve { //: public SFSGeometryCollection {
public:
	virtual bool isClosed()=0;
};

class SFSMultiLineString : public SFSMultiCurve {
};

class SFSMultiPoint { //: public SFSGeometryCollection {
};

class SFSMultiSurface { //: public SFSGeometryCollection {
};

class SFSMultiPolygon : public SFSMultiSurface {
};

class SFSPoint {// : public SFSGeometry {
public:
	virtual double getX()=0;
	virtual double getY()=0;
	virtual Coordinate* getCoordinate()=0;
};

class Point : public Geometry, public SFSPoint {
public:
	Point(void);
	Point(Coordinate c, PrecisionModel precisionModel, int SRID);
	Point(const Point &p); //replaces clone()
	virtual ~Point(void);
	CoordinateList* getCoordinates(void);
	int getNumPoints();
	bool isEmpty();
	bool isSimple();
	bool isValid();
	int getDimension();
	int getBoundaryDimension();
	double getX();
	double getY();
	Coordinate* getCoordinate();
	string getGeometryType();
	Geometry* getBoundary();
	void apply(CoordinateFilter *filter);
	void apply(GeometryFilter *filter);
	void apply(GeometryComponentFilter *filter);
	bool equalsExact(Geometry *other);
	void normalize(void) { };
protected:
	Coordinate coordinate;
	Envelope* computeEnvelopeInternal();
	int compareToSameClass(Point *p);
};

class  SFSCurve { //: public SFSGeometry {
public:
	virtual Point* getStartPoint()=0;
	virtual Point* getEndPoint()=0;
	virtual bool isClosed()=0;
	virtual bool isRing()=0;
	virtual bool isSimple()=0;
};

class SFSLineString : public SFSCurve  {
public:
	virtual int getNumPoints()=0;
	virtual Point* getPointN(int n)=0;
	virtual Coordinate getCoordinateN(int n)=0;

};

class SFSLinearRing { // : public SFSLineString { //For some reason generates 'virtual function not impl.'
};

class LineString: public Geometry, public SFSLineString {
public:
	LineString();
	LineString(const LineString &ls);
	LineString(CoordinateList* newPoints, PrecisionModel* precisionModel, int SRID);
	virtual ~LineString();
	virtual CoordinateList* getCoordinates();
	virtual Coordinate getCoordinateN(int n);
	virtual int getDimension();
	virtual int getBoundaryDimension();
	virtual bool isEmpty();
	virtual int getNumPoints();
	virtual Point* getPointN(int n);
	virtual Point* getStartPoint();
	virtual Point* getEndPoint();
	virtual bool isClosed();
	virtual bool isRing();
	virtual string getGeometryType();
	virtual bool isSimple();
	virtual Geometry* getBoundary();
	virtual bool isCoordinate(Coordinate& pt);
	virtual bool equalsExact(Geometry *other);
	virtual void apply(CoordinateFilter *filter);
	virtual void apply(GeometryFilter *filter);
	virtual void apply(GeometryComponentFilter *filter);
	virtual void normalize();
	virtual int compareToSameClass(LineString *ls); //was protected
	virtual Coordinate* getCoordinate();
	virtual double getLength();
protected:
	CoordinateList* points;
	virtual Envelope* computeEnvelopeInternal();
	virtual bool isEquivalentClass(Geometry *other);
};

class LinearRing : public LineString, public SFSLinearRing {
public:
	LinearRing();
	LinearRing(const LinearRing &lr);
	LinearRing(CoordinateList* points,PrecisionModel* precisionModel,int SRID);
	virtual ~LinearRing();
	bool isSimple();
	string getGeometryType();
	bool isClosed();
	void setPoints(CoordinateList* cl);
};

class SFSSurface { //: public SFSGeometry {
};

class SFSPolygon : public SFSSurface {
public:
	virtual LineString *getExteriorRing()=0;
	virtual int getNumInteriorRing()=0;
	virtual LineString *getInteriorRingN(int n)=0;
};

class Polygon: public Geometry, public SFSPolygon  {
public:
	Polygon();
	Polygon(const Polygon &p);
	virtual ~Polygon();
	Polygon(LinearRing *newShell, PrecisionModel* precisionModel, int SRID);
	Polygon(LinearRing *newShell, vector<Geometry *> *newHoles, PrecisionModel* precisionModel, int SRID);
	CoordinateList* getCoordinates();
	int getNumPoints();
	int getDimension();
	int getBoundaryDimension();
	bool isEmpty();
	bool isSimple();
	LineString *getExteriorRing();
	int getNumInteriorRing();
	LineString *getInteriorRingN(int n);
	string getGeometryType();
	Geometry* getBoundary();
	bool equalsExact(Geometry *other);
	void apply(CoordinateFilter *filter);
	void apply(GeometryFilter *filter);
	Geometry convexHull();
	void normalize();
	int compareToSameClass(Polygon *p); //was protected
	Coordinate* getCoordinate();
	double getArea();
	double getLength();
	void apply(GeometryComponentFilter *filter);
protected:
	LinearRing *shell;
	vector<Geometry *> holes; //Actually vector<LinearRing *>
	Envelope* computeEnvelopeInternal();
private:
	void normalize(LinearRing *ring, bool clockwise);
};

class MultiPoint: public GeometryCollection, public SFSMultiPoint {
public:
	MultiPoint();
	MultiPoint(vector<Geometry *> *points,PrecisionModel pm, int SRID);
	virtual ~MultiPoint();
	int getDimension();
	int getBoundaryDimension();
	string getGeometryType();
	bool isValid();
	bool isClosed();
	bool isSimple();
	Geometry* getBoundary();
	bool equalsExact(Geometry *other);
protected:
	Coordinate* getCoordinate(int n);
};

class MultiLineString: public GeometryCollection, public SFSMultiLineString  {
public:
	MultiLineString();
	MultiLineString(vector<Geometry *> *lineStrings, PrecisionModel precisionModel, int SRID);
	virtual ~MultiLineString();
	int getDimension();
	int getBoundaryDimension();
	string getGeometryType();
	bool isClosed();
	bool isSimple();
	Geometry* getBoundary();
	bool equalsExact(Geometry *other);
};

class MultiPolygon: public GeometryCollection, public SFSMultiPolygon   {
public:
	MultiPolygon();
	MultiPolygon(vector<Geometry *> *polygons, PrecisionModel precisionModel, int SRID);
	virtual ~MultiPolygon();
	int getDimension();
	int getBoundaryDimension();
	string getGeometryType();
	bool isSimple();
	Geometry* getBoundary();
	bool equalsExact(Geometry *other);
};

class GeometryFactory {
public:
	GeometryFactory();
	GeometryFactory(PrecisionModel* newPrecisionModel, int newSRID);
	virtual ~GeometryFactory();

//Skipped a lot of list to array convertors

	static Geometry* toGeometry(Envelope* envelope,PrecisionModel* precisionModel,int SRID);
	PrecisionModel* getPrecisionModel();
	Point* createPoint(Coordinate coordinate);
	MultiLineString* createMultiLineString(vector<Geometry *> *lineStrings);
	GeometryCollection* createGeometryCollection(vector<Geometry *> *geometries);
	MultiPolygon* createMultiPolygon(vector<Geometry *> *polygons);
	LinearRing* createLinearRing(CoordinateList* coordinates);
	MultiPoint* createMultiPoint(vector<Geometry *> *point);
	MultiPoint* createMultiPoint(CoordinateList* coordinates);
	Polygon* createPolygon(LinearRing *shell, vector<Geometry *> *holes);
	LineString* createLineString(CoordinateList* coordinates);
	Geometry* buildGeometry(vector<Geometry *> *geoms);
private:
	PrecisionModel* precisionModel;
	int SRID;
};

#endif
