/*
* $Log$
* Revision 1.37  2003/10/15 16:39:03  strk
* Made Edge::getCoordinates() return a 'const' value. Adapted code set.
*
* Revision 1.36  2003/10/15 11:23:00  strk
* Formalized const nature of toVector() method and of first argument to static removeRepeatedPoints().
*
* Revision 1.35  2003/10/15 10:17:36  strk
* Made setPoints() get a const vector<Coordinate>.
*
* Revision 1.34  2003/10/15 09:54:29  strk
* Added getCoordinatesRO() public method.
*
* Revision 1.33  2003/10/13 12:51:28  strk
* removed sortedClasses strings array from all geometries.
*
* Revision 1.32  2003/10/11 01:56:08  strk
*
* Code base padded with 'const' keywords ;)
*
* Revision 1.31  2003/10/09 15:35:13  strk
* added 'const' keyword to GeometryFactory constructor, Log on top of geom.h
*
*/
#ifndef GEOS_GEOM_H
#define GEOS_GEOM_H

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "math.h"
//#include "util.h"
#include "platform.h"

using namespace std;

namespace geos {

class Coordinate;

/**
*  Converts a coordinate to and from a "precise" coordinate; that is, one whose
*  precision is known exactly. In other words, specifies the grid of allowable
*  points for all <code>Geometry</code>s. <P>
*
*  Vertices are assumed to be precise in JTS. That is, the coordinates of
*  vertices are assumed to be rounded to the defined precision model. Input
*  routines will be responsible for rounding coordinates to the precision model
*  before creating JTS structures. Non-constructive internal operations will
*  assume that coordinates are rounded to the precision model. <P>
*
*  JTS methods do not handle inputs with different precision models. <P>
*
*  The Precision Model is be specified by a scale factor.
*  The scale factor specifies the grid which numbers are rounded to.
*  World coordinates are mapped to JTS coordinates according to the following
*  equations:
*  <UL>
*    <LI> jtsPt.x = round( (inputPt.x * scale ) / scale
*    <LI> jtsPt.y = round( (inputPt.y * scale ) / scale
*  </UL>
*  Coordinates are be represented as double-precision values.
* Since Java uses the IEEE-394 floating point standard, this

*  provides 53 bits of precision. (Thus the maximum precisely representable
*  integer is 9,007,199,254,740,992).
*
*@version 1.3
*/
class PrecisionModel {
public:
	enum {
		FIXED=1,
		FLOATING
	};
	static const double maximumPreciseValue;
	double makePrecise(double val) const;
	void makePrecise(Coordinate *coord) const;
	PrecisionModel(void);
	PrecisionModel(double newScale, double newOffsetX, double newOffsetY);
	PrecisionModel(double newScale);
	PrecisionModel(const PrecisionModel &pm);
	virtual ~PrecisionModel(void);
	bool isFloating() const;
	double getScale() const;
	double getOffsetX() const;
	double getOffsetY() const;
	void toInternal(const Coordinate& external, Coordinate* internal) const;
	Coordinate* toInternal(const Coordinate& external) const;
	Coordinate* toExternal(const Coordinate& internal) const;
	void toExternal(const Coordinate& internal, Coordinate* external) const;
	string toString() const;
	int compareTo(const PrecisionModel* other) const;
private:
	void setScale(double newScale);
	int modelType;
	double scale;
	double offsetX;
	double offsetY;
};

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
	//void setNull(void);
	//static Coordinate& getNull(void);
	virtual ~Coordinate(){};
	//Coordinate(double xNew, double yNew, double zNew);
	//Coordinate(const Coordinate& c);
	//Coordinate(double xNew, double yNew);
	//void setCoordinate(Coordinate& other);
	//bool equals2D(Coordinate& other);
	//int compareTo(Coordinate& other);
	//bool equals3D(Coordinate& other);
	string toString() const;
	//void makePrecise();
	//double distance(Coordinate& p);
	static Coordinate nullCoord;

	void Coordinate::setNull() {
		x=DoubleNotANumber;
		y=DoubleNotANumber;
		z=DoubleNotANumber;
	}

	static Coordinate& Coordinate::getNull() {
		return nullCoord;
	}

	Coordinate::Coordinate() {
		x=0.0;
		y=0.0;
		z=DoubleNotANumber;
	}

	Coordinate::Coordinate(double xNew, double yNew, double zNew) {
		x=xNew;
		y=yNew;
		z=zNew;
	}

	Coordinate::Coordinate(const Coordinate& c){
		x=c.x;
		y=c.y;
		z=c.z;
	}

	Coordinate::Coordinate(double xNew, double yNew){
		x=xNew;
		y=yNew;
		z=DoubleNotANumber;
	}

	void Coordinate::setCoordinate(const Coordinate& other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}

	bool Coordinate::equals2D(const Coordinate& other) const {
		if (x != other.x) {
		return false;
		}
		if (y != other.y) {
		return false;
		}
		return true;
	}

	int Coordinate::compareTo(const Coordinate& other) const {
		if (x < other.x) {
		return -1;
		}
		if (x > other.x) {
		return 1;
		}
		if (y < other.y) {
		return -1;
		}
		if (y > other.y) {
		return 1;
		}
		return 0;
	}

	bool Coordinate::equals3D(const Coordinate& other) const {
		return (x == other.x) && ( y == other.y) && (( z == other.z)||(z==DoubleNotANumber && other.z==DoubleNotANumber));
	}

	void Coordinate::makePrecise(const PrecisionModel *precisionModel) {
		x = precisionModel->makePrecise(x);
		y = precisionModel->makePrecise(y);
	}

	double Coordinate::distance(const Coordinate& p) const {
		double dx = x - p.x;
		double dy = y - p.y;
		return sqrt(dx * dx + dy * dy);
	}

	double x;	/// x-coordinate
	double y;	/// y-coordinate
	double z;	/// z-coordinate

};

class CoordinateList {
public:
	virtual	bool isEmpty() const=0;
	virtual	void add(const Coordinate& c)=0;
	virtual	int getSize() const=0;
	virtual	const Coordinate& getAt(int pos) const=0;
	virtual	void setAt(const Coordinate& c, int pos)=0;
	virtual	void deleteAt(int pos)=0;
	virtual	vector<Coordinate>* toVector() const=0;
	virtual	string toString()=0;
	virtual	void setPoints(const vector<Coordinate> &v)=0;
	bool hasRepeatedPoints() const;
	const Coordinate* minCoordinate() const;
	static bool hasRepeatedPoints(const CoordinateList *cl);
	static const Coordinate* minCoordinate(CoordinateList *cl);
	static int indexOf(const Coordinate *coordinate, const CoordinateList *cl);
	static bool equals(CoordinateList *cl1, CoordinateList *cl2);
	static void scroll(CoordinateList *cl, const Coordinate *firstCoordinate);
	static void reverse(CoordinateList *cl);
	void add(vector<Coordinate>* vc,bool allowRepeated);
	void add(const Coordinate& c,bool allowRepeated);
	static CoordinateList* removeRepeatedPoints(const CoordinateList *cl);
	virtual ~CoordinateList(){};
};

class BasicCoordinateList : public CoordinateList {
public:
	BasicCoordinateList();
	BasicCoordinateList(int n);
	BasicCoordinateList(const Coordinate& c);
	BasicCoordinateList(const BasicCoordinateList &cl);
	BasicCoordinateList(const CoordinateList *c);
	virtual ~BasicCoordinateList();
	bool isEmpty() const;
	void add(const Coordinate& c);
	int getSize() const;
	const Coordinate& getAt(int pos) const;
	void setAt(const Coordinate& c, int pos);
	void deleteAt(int pos);
	vector<Coordinate>* toVector() const;
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
	PointCoordinateList(const Coordinate& c);
	PointCoordinateList(const PointCoordinateList &cl);
	PointCoordinateList(const CoordinateList *c);
	virtual ~PointCoordinateList();
	bool isEmpty() const;
	void add(const Coordinate& c);
	void add(point_3d p);
	int getSize() const;
	const Coordinate& getAt(int pos) const;
	point_3d getPointAt(int pos);
	void setAt(const Coordinate& c, int pos);
	void setAt(point_3d p, int pos);
	void deleteAt(int pos);
	vector<Coordinate>* toVector() const;
	vector<point_3d>* toPointVector();
	string toString();
	void setPoints(const vector<Coordinate> &v);
	void setPoints(vector<point_3d> &v);
private:
	vector<point_3d> *vect;
};

class CoordinateListFactory {
public:
	virtual CoordinateList* createCoordinateList()=0;
	virtual CoordinateList* createCoordinateList(int size)=0;
	virtual CoordinateList* createCoordinateList(const Coordinate& c)=0;
	virtual CoordinateList* createCoordinateList(const CoordinateList *c)=0;
	static CoordinateListFactory* internalFactory;
};

class BasicCoordinateListFactory: public CoordinateListFactory {
	CoordinateList* createCoordinateList() {return new BasicCoordinateList();};
	CoordinateList* createCoordinateList(int size) {return new BasicCoordinateList(size);};
	CoordinateList* createCoordinateList(const Coordinate& c) {return new BasicCoordinateList(c);};
	CoordinateList* createCoordinateList(const CoordinateList *c) {return new BasicCoordinateList(c);};
};

class PointCoordinateListFactory: public CoordinateListFactory {
	CoordinateList* createCoordinateList() {return new PointCoordinateList();};
	CoordinateList* createCoordinateList(int size) {return new PointCoordinateList(size);};
	CoordinateList* createCoordinateList(const Coordinate& c) {return new PointCoordinateList(c);};
	CoordinateList* createCoordinateList(const CoordinateList *c) {return new PointCoordinateList(c);};
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
   virtual void filter_rw(Coordinate& coord)=0;
   virtual void filter_ro(const Coordinate& coord)=0;
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
//	virtual void filter(Geometry *geom)=0;
	virtual void filter_rw(Geometry *geom);
	virtual void filter_ro(const Geometry *geom); // Unsupported
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
	Envelope(const Coordinate& p1, const Coordinate& p2);
	Envelope(const Coordinate& p);
	Envelope(const Envelope &env);
	virtual ~Envelope(void);
	static bool intersects(const Coordinate& p1,const Coordinate& p2,const Coordinate& q);
	static bool intersects(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2);
	void init(void);
	void init(double x1, double x2, double y1, double y2);
	void init(const Coordinate& p1, const Coordinate& p2);
	void init(const Coordinate& p);
	void init(Envelope env);
	void setToNull(void);
	bool isNull(void) const;
	double getWidth(void) const;
	double getHeight(void) const;
	double getMaxY() const;
	double getMaxX() const;
	double getMinY() const;
	double getMinX() const;
	void expandToInclude(const Coordinate& p);
	void expandToInclude(double x, double y);
	void expandToInclude(const Envelope* other);
	bool contains(const Coordinate& p) const;
	bool contains(double x, double y) const;
	bool contains(const Envelope* other) const;
	bool overlaps(const Coordinate& p) const;
	bool overlaps(double x, double y) const;
	bool overlaps(const Envelope* other) const;
	bool intersects(const Coordinate& p) const;
	bool intersects(double x, double y) const;
	bool intersects(const Envelope* other) const;
	string toString(void) const;
	double distance(const Envelope* env) const;
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
	virtual int getSRID() const=0;
	virtual string getGeometryType() const=0;
	virtual PrecisionModel* getPrecisionModel() const=0;
	virtual Envelope* getEnvelopeInternal() const=0;
	virtual Geometry* getEnvelope()=0;
	virtual bool isEmpty() const=0;
	virtual bool isSimple() const=0;
	virtual Geometry* getBoundary() const=0;
	virtual int getDimension() const=0;
	virtual bool equals(Geometry *other)=0;
	virtual bool disjoint(Geometry *other)=0;
	virtual bool intersects(Geometry *other)=0;
	virtual bool touches(Geometry *other)=0;
	virtual bool crosses(Geometry *other)=0;
	virtual bool within(Geometry *other)=0;
	virtual bool contains(Geometry *other)=0;
	virtual bool overlaps(Geometry *other)=0;
	virtual bool relate(Geometry *other, string intersectionPattern)=0;
	virtual IntersectionMatrix* relate(Geometry *other)=0;
	virtual Geometry* buffer(double distance)=0;
	virtual Geometry* convexHull() const=0;
	virtual Geometry* intersection(const Geometry *other)=0;
	virtual Geometry* Union(Geometry *other)=0;
	virtual Geometry* difference(Geometry *other)=0;
	virtual Geometry* symDifference(Geometry *other)=0;
	virtual string toText()=0;
};

class CGAlgorithms;
class Point;
class Geometry: public SFSGeometry {
friend class Unload;
public:
	void throw_exception ();
	Geometry(void);
	Geometry(const Geometry &geom);
	Geometry(const PrecisionModel* pm, int SRID);
	virtual string getGeometryType() const=0; //Abstract
	virtual int getSRID() const;
	virtual void setSRID(int newSRID);
	virtual PrecisionModel* getPrecisionModel() const;
	virtual const Coordinate* getCoordinate() const=0; //Abstract
	virtual CoordinateList* getCoordinates() const=0; //Abstract
	virtual int getNumPoints() const=0; //Abstract
	virtual bool isSimple() const=0; //Abstract
	virtual bool isValid() const;
	virtual bool isEmpty() const=0; //Abstract
	virtual int getDimension() const=0; //Abstract
	virtual Geometry* getBoundary() const=0; //Abstract
	virtual int getBoundaryDimension() const=0; //Abstract
	virtual Geometry* getEnvelope();
	virtual Envelope* getEnvelopeInternal() const;
	virtual bool disjoint(Geometry *g);
	virtual bool touches(Geometry *g);
	virtual bool intersects(Geometry *g);
	virtual bool crosses(Geometry *g);
	virtual bool within(Geometry *g);
	virtual bool contains(Geometry *g);
	virtual bool overlaps(Geometry *g);
	virtual bool relate(Geometry *g, string intersectionPattern);
	virtual IntersectionMatrix* relate(Geometry *g);
	virtual bool equals(Geometry *g);
	virtual string toString();
	virtual string toText();
	virtual Geometry* buffer(double distance);
	virtual Geometry* buffer(double distance,int quadrantSegments);
	virtual Geometry* convexHull() const;
	virtual Geometry* intersection(const Geometry *other);
	virtual Geometry* Union(Geometry *other);
	virtual Geometry* difference(Geometry *other);
	virtual Geometry* symDifference(Geometry *other);
	virtual bool equalsExact(const Geometry *other, double tolerance) const=0; //Abstract
	virtual void apply_rw(CoordinateFilter *filter)=0; //Abstract
	virtual void apply_ro(CoordinateFilter *filter) const=0; //Abstract
	virtual void apply_rw(GeometryFilter *filter);
	virtual void apply_ro(GeometryFilter *filter) const;
	virtual void apply_rw(GeometryComponentFilter *filter);
	virtual void apply_ro(GeometryComponentFilter *filter) const;
	//public Object clone() // Replaced by copy constructor
	virtual void normalize()=0; //Abstract
	virtual int compareTo(const Geometry *geom) const;
	virtual double distance(const Geometry *g) const;
	virtual double getArea() const;
	virtual double getLength() const;
	virtual ~Geometry();
	virtual bool isWithinDistance(Geometry *geom,double cDistance);
	virtual Point* getCentroid() const;
	virtual Point* getInteriorPoint();
	virtual void geometryChanged();
	void geometryChangedAction();
protected:
	PrecisionModel* precisionModel;
	int SRID;
	Envelope* envelope;
	static CGAlgorithms *cgAlgorithms;
	static bool hasNonEmptyElements(vector<Geometry *>* geometries);
	static bool hasNullElements(const CoordinateList* list);
	static bool hasNullElements(vector<Geometry *>* lrs);
//	static void reversePointOrder(CoordinateList* coordinates);
//	static Coordinate& minCoordinate(CoordinateList* coordinates);
//	static void scroll(CoordinateList* coordinates,Coordinate* firstCoordinate);
//	static int indexOf(Coordinate* coordinate,CoordinateList* coordinates);
	virtual bool isEquivalentClass(const Geometry *other) const;
	virtual void checkNotGeometryCollection(const Geometry *g) const;
	//virtual void checkEqualSRID(Geometry *other);
	//virtual void checkEqualPrecisionModel(Geometry *other);
	virtual Envelope* computeEnvelopeInternal() const=0; //Abstract
	virtual int compareToSameClass(const Geometry *geom) const=0; //Abstract
	int compare(vector<Coordinate> a, vector<Coordinate> b) const;
	int compare(vector<Geometry *> a, vector<Geometry *> b) const;
	bool equal(const Coordinate& a, const Coordinate& b,double tolerance) const;
private:
	virtual int getClassSortIndex() const;
	static GeometryComponentFilter* geometryChangedFilter;
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
	virtual void filter_ro(const Geometry *geom)=0;
	virtual void filter_rw(Geometry *geom)=0;
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
	LineSegment(const Coordinate& c0, const Coordinate& c1);
	virtual ~LineSegment(void);
	virtual void setCoordinates(const Coordinate& c0, const Coordinate& c1);
	virtual const Coordinate& getCoordinate(int i) const;
	virtual void setCoordinates(const LineSegment ls);
	virtual double getLength() const;
	virtual void reverse();
	virtual void normalize();
	virtual double angle() const;
	virtual double distance(const LineSegment ls) const;
	virtual double distance(const Coordinate& p) const;
	virtual double projectionFactor(const Coordinate& p) const;
	virtual Coordinate* project(const Coordinate& p) const;
	virtual LineSegment* project(const LineSegment *seg) const;
	virtual Coordinate* closestPoint(const Coordinate& p) const;
	virtual int compareTo(const LineSegment other) const;
	virtual bool equalsTopo(const LineSegment other) const;
	virtual string toString() const;
};

class IntersectionMatrix {
public:
	IntersectionMatrix();
	IntersectionMatrix(string elements);
	IntersectionMatrix(const IntersectionMatrix &im);
	virtual ~IntersectionMatrix();
	static bool matches(int actualDimensionValue, char requiredDimensionSymbol);
	static bool matches(string actualDimensionSymbols, string requiredDimensionSymbols);
	void add(IntersectionMatrix *im);
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

/**
 * Indicates an invalid or inconsistent topological situation encountered during processing
 */
class TopologyException {
private:
	static string msgWithCoord(string msg,Coordinate *newPt);
	Coordinate *pt;
public:
	TopologyException(string msg);
	TopologyException(string msg,Coordinate *newPt);
	Coordinate* getCoordinate();
	void setName(string nname);
	void setMessage(string msg);
protected:
	string txt;
	string name;
};

//Operators
bool operator==(const Coordinate& a, const Coordinate& b);
bool operator==(const Envelope a, const Envelope b);
bool operator==(const PrecisionModel a, const PrecisionModel b);
bool operator==(const LineSegment a, const LineSegment b);

bool lessThen(Coordinate& a,Coordinate& b);
bool greaterThen(Geometry *first, Geometry *second);

class SFSGeometryCollection { //: public SFSGeometry {
public:
	virtual int getNumGeometries() const=0;
	virtual const Geometry* getGeometryN(int n) const=0;
};

class GeometryCollection : public Geometry, public SFSGeometryCollection {
public:
	GeometryCollection(void);
	GeometryCollection(const GeometryCollection &gc);
	GeometryCollection(vector<Geometry *> *newGeometries,PrecisionModel* pm, int SRID);
	virtual ~GeometryCollection();
	virtual CoordinateList* getCoordinates() const;
	virtual bool isEmpty() const;
	virtual int getDimension() const;
	virtual int getBoundaryDimension() const;
	virtual int getNumGeometries() const;
	virtual const Geometry* getGeometryN(int n) const;
	virtual int getNumPoints() const;
	virtual string getGeometryType() const;
	virtual bool isSimple() const;
	virtual Geometry* getBoundary() const;
	virtual bool equalsExact(const Geometry *other, double tolerance) const;
	virtual void apply_ro(CoordinateFilter *filter) const;
	virtual void apply_rw(CoordinateFilter *filter);
	virtual void apply_ro(GeometryFilter *filter) const;
	virtual void apply_rw(GeometryFilter *filter);
	virtual void apply_ro(GeometryComponentFilter *filter) const;
	virtual void apply_rw(GeometryComponentFilter *filter);
	virtual void normalize();
	virtual const Coordinate* getCoordinate() const;
	virtual double getArea() const;
	virtual double getLength() const;
	virtual Point* getCentroid() const;
protected:
	vector<Geometry *>* geometries;
	virtual Envelope* computeEnvelopeInternal() const;
	virtual int compareToSameClass(const Geometry *gc) const;
};

class GeometryCollectionIterator {
public:
	GeometryCollectionIterator();
	GeometryCollectionIterator(const GeometryCollectionIterator &gci);
	GeometryCollectionIterator(const GeometryCollection *newParent);
	virtual ~GeometryCollectionIterator();
	bool hasNext() const;
	const Geometry *next();
	void remove(); //Not implemented
private:
	const GeometryCollection* parent;
	bool atStart;
	int max;
	int index;
	GeometryCollectionIterator* subcollectionIterator;
};

class SFSMultiCurve { //: public SFSGeometryCollection {
public:
	virtual bool isClosed() const=0;
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
	virtual double getX() const=0;
	virtual double getY() const=0;
	virtual const Coordinate* getCoordinate() const=0;
};

class Point : public Geometry, public SFSPoint {
public:
	Point(void);
	Point(const Coordinate& c, const PrecisionModel* pm, int SRID);
	Point(const Point &p); //replaces clone()
	virtual ~Point();
	CoordinateList* getCoordinates(void) const;
	int getNumPoints() const;
	bool isEmpty() const;
	bool isSimple() const;
	bool isValid() const;
	int getDimension() const;
	int getBoundaryDimension() const;
	double getX() const;
	double getY() const;
	const Coordinate* getCoordinate() const;
	string getGeometryType() const;
	Geometry* getBoundary() const;
	void apply_ro(CoordinateFilter *filter) const;
	void apply_rw(CoordinateFilter *filter);
	void apply_ro(GeometryFilter *filter) const;
	void apply_rw(GeometryFilter *filter);
	void apply_rw(GeometryComponentFilter *filter);
	void apply_ro(GeometryComponentFilter *filter) const;
	bool equalsExact(const Geometry *other, double tolerance) const;
	void normalize(void) { };
protected:
	Coordinate coordinate;
	Envelope* computeEnvelopeInternal() const;
	int compareToSameClass(const Geometry *p) const;
};

class  SFSCurve { //: public SFSGeometry {
public:
	virtual Point* getStartPoint() const=0;
	virtual Point* getEndPoint() const=0;
	virtual bool isClosed() const=0;
	virtual bool isRing() const=0;
	virtual bool isSimple() const=0;
};

class SFSLineString : public SFSCurve  {
public:
	virtual int getNumPoints() const=0;
	virtual Point* getPointN(int n) const=0;
	virtual const Coordinate& getCoordinateN(int n) const=0;

};

class SFSLinearRing { // : public SFSLineString { //For some reason generates 'virtual function not impl.'
};

class LineString: public Geometry, public SFSLineString {
public:
	LineString();
	LineString(const LineString &ls);
	LineString(const CoordinateList *pts, const PrecisionModel *pm, int SRID);
	virtual ~LineString();
	virtual CoordinateList* getCoordinates() const;
	const CoordinateList* getCoordinatesRO() const;
	virtual const Coordinate& getCoordinateN(int n) const;
	virtual int getDimension() const;
	virtual int getBoundaryDimension() const;
	virtual bool isEmpty() const;
	virtual int getNumPoints() const;
	virtual Point* getPointN(int n) const;
	virtual Point* getStartPoint() const;
	virtual Point* getEndPoint() const;
	virtual bool isClosed() const;
	virtual bool isRing() const;
	virtual string getGeometryType() const;
	virtual bool isSimple() const;
	virtual Geometry* getBoundary() const;
	virtual bool isCoordinate(Coordinate& pt) const;
	virtual bool equalsExact(const Geometry *other, double tolerance) const;
	virtual void apply_rw(CoordinateFilter *filter);
	virtual void apply_ro(CoordinateFilter *filter) const;
	virtual void apply_rw(GeometryFilter *filter);
	virtual void apply_ro(GeometryFilter *filter) const;
	virtual void apply_rw(GeometryComponentFilter *filter);
	virtual void apply_ro(GeometryComponentFilter *filter) const;
	virtual void normalize();
	//was protected
	virtual int compareToSameClass(const Geometry *ls) const;
	virtual int compareTo(const LineString *ls) const;
	virtual const Coordinate* getCoordinate() const;
	virtual double getLength() const;
protected:
	CoordinateList* points;
	virtual Envelope* computeEnvelopeInternal() const;
	virtual bool isEquivalentClass(const Geometry *other) const;
};

class LinearRing : public LineString, public SFSLinearRing {
public:
	LinearRing();
	LinearRing(const LinearRing &lr);
	LinearRing(const CoordinateList* points, const PrecisionModel* pm, int SRID);
	virtual ~LinearRing();
	bool isSimple() const;
	string getGeometryType() const;
	bool isClosed() const;
	void setPoints(CoordinateList* cl);
};

class SFSSurface { //: public SFSGeometry {
};

class SFSPolygon : public SFSSurface {
public:
	virtual const LineString* getExteriorRing() const=0;
	virtual int getNumInteriorRing() const=0;
	virtual const LineString* getInteriorRingN(int n) const=0;
};

class Polygon: public Geometry, public SFSPolygon  {
public:
	Polygon();
	Polygon(const Polygon &p);
	virtual ~Polygon();
	Polygon(LinearRing *newShell, PrecisionModel* precisionModel, int SRID);
	Polygon(LinearRing *newShell, vector<Geometry *> *newHoles, PrecisionModel* precisionModel, int SRID);
	CoordinateList* getCoordinates() const;
	int getNumPoints() const;
	int getDimension() const;
	int getBoundaryDimension() const;
	bool isEmpty() const;
	bool isSimple() const;
	const LineString* getExteriorRing() const;
	int getNumInteriorRing() const;
	const LineString* getInteriorRingN(int n) const;
	string getGeometryType() const;
	Geometry* getBoundary() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
	void apply_rw(CoordinateFilter *filter);
	void apply_ro(CoordinateFilter *filter) const;
	void apply_rw(GeometryFilter *filter);
	void apply_ro(GeometryFilter *filter) const;
	Geometry* convexHull() const;
	void normalize();
	int compareToSameClass(const Geometry *p) const; //was protected
	const Coordinate* getCoordinate() const;
	double getArea() const;
	double getLength() const;
	void apply_rw(GeometryComponentFilter *filter);
	void apply_ro(GeometryComponentFilter *filter) const;
protected:
	LinearRing *shell;
	vector<Geometry *> *holes; //Actually vector<LinearRing *>
	Envelope* computeEnvelopeInternal() const;
private:
	void normalize(LinearRing *ring, bool clockwise);
};

class MultiPoint: public GeometryCollection, public SFSMultiPoint {
public:
	MultiPoint();
	MultiPoint(vector<Geometry *> *points,PrecisionModel* pm, int SRID);
	virtual ~MultiPoint();
	int getDimension() const;
	int getBoundaryDimension() const;
	string getGeometryType() const;
	bool isValid() const;
	bool isClosed() const;
	bool isSimple() const;
	Geometry* getBoundary() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
protected:
	const Coordinate* getCoordinate(int n) const;
};

class MultiLineString: public GeometryCollection, public SFSMultiLineString  {
public:
	MultiLineString();
	MultiLineString(vector<Geometry *> *lineStrings, PrecisionModel* precisionModel, int SRID);
	virtual ~MultiLineString();
	int getDimension() const;
	int getBoundaryDimension() const;
	string getGeometryType() const;
	bool isClosed() const;
	bool isSimple() const;
	Geometry* getBoundary() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
};

class MultiPolygon: public GeometryCollection, public SFSMultiPolygon   {
public:
	MultiPolygon();
	MultiPolygon(vector<Geometry *> *polygons, PrecisionModel* precisionModel, int SRID);
	virtual ~MultiPolygon();
	int getDimension() const;
	int getBoundaryDimension() const;
	string getGeometryType() const;
	bool isSimple() const;
	Geometry* getBoundary() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
};

class GeometryFactory {
public:
	GeometryFactory();
	GeometryFactory(const PrecisionModel* pm, int newSRID);
	virtual ~GeometryFactory();

//Skipped a lot of list to array convertors

	static Point* createPointFromInternalCoord(const Coordinate* coord, const Geometry *exemplar);
	static Geometry* toGeometry(Envelope* envelope,PrecisionModel* precisionModel,int SRID);
	PrecisionModel* getPrecisionModel() const;
	Point* createPoint(const Coordinate& coordinate);
	MultiLineString* createMultiLineString(vector<Geometry *> *lineStrings);
	GeometryCollection* createGeometryCollection(vector<Geometry *> *geometries);
	MultiPolygon* createMultiPolygon(vector<Geometry *> *polygons);
	LinearRing* createLinearRing(CoordinateList* coordinates);
	MultiPoint* createMultiPoint(vector<Geometry *> *point);
	MultiPoint* createMultiPoint(CoordinateList* coordinates);
	Polygon* createPolygon(LinearRing *shell, vector<Geometry *> *holes);
	LineString* createLineString(const CoordinateList* coordinates);
	Geometry* buildGeometry(vector<Geometry *> *geoms);
private:
	PrecisionModel* precisionModel;
	int SRID;
};
}
#endif
