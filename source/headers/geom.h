/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.63  2004/05/17 21:14:47  ybychkov
 * JavaDoc updated
 *
 * Revision 1.62  2004/05/17 21:03:56  ybychkov
 * JavaDoc updated
 *
 * Revision 1.61  2004/05/14 12:14:08  strk
 * const correctness
 *
 * Revision 1.60  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateList.
 *
 * Revision 1.59  2004/05/05 10:54:48  strk
 * Removed some private static heap explicit allocation, less cleanup done by
 * the unloader.
 *
 * Revision 1.58  2004/04/20 08:52:01  strk
 * GeometryFactory and Geometry const correctness.
 * Memory leaks removed from SimpleGeometryPrecisionReducer
 * and GeometryFactory.
 *
 * Revision 1.57  2004/04/16 08:35:52  strk
 * Memory leaks fixed and const correctness applied for Point class.
 *
 * Revision 1.56  2004/04/16 07:42:06  strk
 * PrecisionModel::Type made an enum instead of a Type.
 *
 * Revision 1.55  2004/04/15 14:00:30  strk
 * Added new cleanup to Unload::Release
 *
 * Revision 1.54  2004/04/14 13:56:26  strk
 * All geometries returned by {from,to}InternalGeometry calls are
 * now deleted after use (unless NOT new).
 * Some 'commented' throw specifications in geom.h
 *
 * Revision 1.53  2004/04/14 07:29:43  strk
 * Fixed GeometryFactory constructors to copy given PrecisionModel. Added GeometryFactory copy constructor. Fixed Geometry constructors to copy GeometryFactory.
 *
 * Revision 1.52  2004/04/13 08:15:51  strk
 * Changed all 'long long' with int64.
 * Changed all 'long long' constants to end with two Ls.
 *
 * Revision 1.51  2004/04/10 22:41:24  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 * Revision 1.50  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 * Revision 1.49  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 * Revision 1.48  2004/04/01 10:44:34  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.47  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.46  2004/03/29 06:59:24  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.45  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.44  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.43  2004/03/01 22:04:59  strk
 * applied const correctness changes by Manuel Prieto Villegas <ManuelPrietoVillegas@telefonica.net>
 *
 * Revision 1.42  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

#ifndef GEOS_GEOM_H
#define GEOS_GEOM_H

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include "math.h"
//#include "geomUtil.h"
#include "platform.h"

using namespace std;

namespace geos {

enum GeometryTypeId {
	GEOS_POINT,
	GEOS_LINESTRING,
	GEOS_POLYGON,
	GEOS_MULTIPOINT,
	GEOS_MULTILINESTRING,
	GEOS_MULTIPOLYGON,
	GEOS_GEOMETRYCOLLECTION
};

class Coordinate;

/**
 * Specifies the precision model of the {@link Coordinate}s in a {@link Geometry}.
 * In other words, specifies the grid of allowable
 *  points for all <code>Geometry</code>s.
 * <p>
 * The {@link makePrecise} method allows rounding a coordinate to
 * a "precise" value; that is, one whose
 *  precision is known exactly.
 *<p>
 * Coordinates are assumed to be precise in geometries.
 * That is, the coordinates are assumed to be rounded to the
 * precision model given for the geometry.
 * JTS input routines automatically round coordinates to the precision model
 * before creating Geometries.
 * All internal operations
 * assume that coordinates are rounded to the precision model.
 * Constructive methods (such as boolean operations) always round computed
 * coordinates to the appropriate precision model.
 * <p>
 * Currently three types of precision model are supported:
 * <ul>
 * <li>FLOATING - represents full double precision floating point.
 * This is the default precision model used in JTS
 * <li>FLOATING_SINGLE - represents single precision floating point.
 * <li>FIXED - represents a model with a fixed number of decimal places.
 *  A Fixed Precision Model is specified by a scale factor.
 *  The scale factor specifies the grid which numbers are rounded to.
 *  Input coordinates are mapped to fixed coordinates according to the following
 *  equations:
 *    <UL>
 *      <LI> jtsPt.x = round( (inputPt.x * scale ) / scale
 *      <LI> jtsPt.y = round( (inputPt.y * scale ) / scale
 *    </UL>
 * </ul>
 *  Coordinates are represented internally as Java double-precision values.
 * Since Java uses the IEEE-394 floating point standard, this
 *  provides 53 bits of precision. (Thus the maximum precisely representable
 *  integer is 9,007,199,254,740,992).
 *<p>
 *  JTS methods currently do not handle inputs with different precision models.
 *
 */
class PrecisionModel {
friend class Unload;
public:
	/**
	* The types of Precision Model which GEOS supports.
	* <p>
	* This class is only for use to support the "enums" for the types of precision model.
	* <p>
	*/
	typedef enum {
		/**
		* Fixed Precision indicates that coordinates have a fixed number of decimal places.
		* The number of decimal places is determined by the log10 of the scale factor.
		*/
		FIXED,
		/**
		* Floating precision corresponds to the standard Java
		* double-precision floating-point representation, which is
		* based on the IEEE-754 standard
		*/
		FLOATING,
		/**
		* Floating single precision corresponds to the standard Java
		* single-precision floating-point representation, which is
		* based on the IEEE-754 standard
		*/
		FLOATING_SINGLE
	} Type;

  /**
	*  The maximum precise value representable in a double. Since IEE754
	*  double-precision numbers allow 53 bits of mantissa, the value is equal to
	*  2^53 - 1.  This provides <i>almost</i> 16 decimal digits of precision.
	*/
	static const double maximumPreciseValue;
	/**
	* Rounds a numeric value to the PrecisionModel grid.
	*/
	double makePrecise(double val) const;
	void makePrecise(Coordinate *coord) const;
	/**
	* Creates a <code>PrecisionModel</code> with a default precision
	* of FLOATING.
	*/
	PrecisionModel(void);
	PrecisionModel(Type nModelType);
	/**
	*  Creates a <code>PrecisionModel</code> that specifies Fixed precision.
	*  Fixed-precision coordinates are represented as precise internal coordinates,
	*  which are rounded to the grid defined by the scale factor.
	*
	*@param  scale    amount by which to multiply a coordinate after subtracting
	*      the offset, to obtain a precise coordinate
	*@param  offsetX  not used.
	*@param  offsetY  not used.
	*
	* @deprecated offsets are no longer supported, since internal representation is rounded floating point
	*/
	PrecisionModel(double newScale, double newOffsetX, double newOffsetY);
	PrecisionModel(double newScale);
	PrecisionModel(const PrecisionModel &pm);
	virtual ~PrecisionModel(void);
	/**
	* Tests whether the precision model supports floating point
	* @return <code>true</code> if the precision model supports floating point
	*/
	bool isFloating() const;
	/**
	* Returns the maximum number of significant digits provided by this
	* precision model.
	* Intended for use by routines which need to print out precise values.
	*
	* @return the maximum number of decimal places provided by this precision model
	*/
	int getMaximumSignificantDigits() const;
	/**
	* Gets the type of this PrecisionModel
	* @return the type of this PrecisionModel
	*/
	Type getType();
	double getScale() const;
	/**
	* Returns the x-offset used to obtain a precise coordinate.
	*
	* @return the amount by which to subtract the x-coordinate before
	*         multiplying by the scale
	* @deprecated Offsets are no longer used
	*/
	double getOffsetX() const;
	/**
	* Returns the y-offset used to obtain a precise coordinate.
	*
	* @return the amount by which to subtract the y-coordinate before
	*         multiplying by the scale
	* @deprecated Offsets are no longer used
	*/
	double getOffsetY() const;
	/**
	*  Sets <code>internal</code> to the precise representation of <code>external</code>.
	*
	* @param external the original coordinate
	* @param internal the coordinate whose values will be changed to the
	*                 precise representation of <code>external</code>
	* @deprecated use makePrecise instead
	*/
	void toInternal(const Coordinate& external, Coordinate* internal) const;
	/**
	*  Returns the precise representation of <code>external</code>.
	*
	*@param  external  the original coordinate
	*@return           the coordinate whose values will be changed to the precise
	*      representation of <code>external</code>
	* @deprecated use makePrecise instead
	*/
	Coordinate* toInternal(const Coordinate& external) const;
	/**
	*  Returns the external representation of <code>internal</code>.
	*
	*@param  internal  the original coordinate
	*@return           the coordinate whose values will be changed to the
	*      external representation of <code>internal</code>
	* @deprecated no longer needed, since internal representation is same as external representation
	*/
	Coordinate* toExternal(const Coordinate& internal) const;
	/**
	*  Sets <code>external</code> to the external representation of <code>internal</code>
	*  .
	*
	*@param  internal  the original coordinate
	*@param  external  the coordinate whose values will be changed to the
	*      external representation of <code>internal</code>
	* @deprecated no longer needed, since internal representation is same as external representation
	*/
	void toExternal(const Coordinate& internal, Coordinate* external) const;
	string toString() const;
	/**
	*  Compares this {@link PrecisionModel} object with the specified object for order.
	* A PrecisionModel is greater than another if it provides greater precision.
	* The comparison is based on the value returned by the
	* {@link getMaximumSignificantDigits) method.
	* This comparison is not strictly accurate when comparing floating precision models
	* to fixed models; however, it is correct when both models are either floating or fixed.
	*
	*@param  o  the <code>PrecisionModel</code> with which this <code>PrecisionModel</code>
	*      is being compared
	*@return    a negative integer, zero, or a positive integer as this <code>PrecisionModel</code>
	*      is less than, equal to, or greater than the specified <code>PrecisionModel</code>
	*/
	int compareTo(const PrecisionModel* other) const;
private:
	void setScale(double newScale);
	Type modelType;
	double scale;
	static const int64 serialVersionUID = 7777263578777803835LL;
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

	int Coordinate::hashCode() {
        //Algorithm from Effective Java by Joshua Bloch [Jon Aquino]
        int result = 17;
        result = 37 * result + hashCode(x);
        result = 37 * result + hashCode(y);
        return result;
    }

    /**
     * Returns a hash code for a double value, using the algorithm from
     * Joshua Bloch's book <i>Effective Java"</i>
     */
    static int Coordinate::hashCode(double x) {
        int64 f = (int64)(x);
        return (int)(f^(f>>32));
    }


	double x;	/// x-coordinate
	double y;	/// y-coordinate
	double z;	/// z-coordinate
private:
	static const int64 serialVersionUID=6683108902428366910LL;


};


/**
 * The internal representation of a list of coordinates inside a Geometry.
 * <p>
 * There are some cases in which you might want Geometries to store their
 * points using something other than the GEOS Coordinate class. For example, you
 * may want to experiment with another implementation, such as an array of x’s
 * and an array of y’s. or you might want to use your own coordinate class, one
 * that supports extra attributes like M-values.
 * <p>
 * You can do this by implementing the CoordinateList and
 * CoordinateListFactory interfaces. You would then create a
 * GeometryFactory parameterized by your CoordinateListFactory, and use
 * this GeometryFactory to create new Geometries. All of these new Geometries
 * will use your CoordinateList implementation.
 * <p>
 * This class is equivalent to JTS CoordinateSequence.
 */
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
	/**
	* Returns whether #equals returns true for any two consecutive Coordinates 
	* in the given array.
	*/
	static bool hasRepeatedPoints(const CoordinateList *cl);
	/**
	* Returns either the given coordinate array if its length is greater than the
	* given amount, or an empty coordinate array.
	*/
	static CoordinateList* atLeastNCoordinatesOrNothing(int n,CoordinateList *c);
	static const Coordinate* minCoordinate(CoordinateList *cl);
	static int indexOf(const Coordinate *coordinate, const CoordinateList *cl);
	/**
	* Returns true if the two arrays are identical, both null, or pointwise
	* equal (as compared using Coordinate#equals)
	* @see Coordinate#equals(Object)
	*/
	static bool equals(CoordinateList *cl1, CoordinateList *cl2);
	static void scroll(CoordinateList *cl, const Coordinate *firstCoordinate);
	static void reverse(CoordinateList *cl);
	/** Add an array of coordinates
	* @param cl The coordinates
	* @param allowRepeated if set to false, repeated coordinates are collapsed
	* @param direction if false, the array is added in reverse order
	* @return true (as by general collection contract)
	*/
	void add(CoordinateList *cl,bool allowRepeated,bool direction);
	/** Add an array of coordinates
	* @param vc The coordinates
	* @param allowRepeated if set to false, repeated coordinates are collapsed
	* @return true (as by general collection contract)
	*/
	void add(vector<Coordinate>* vc,bool allowRepeated);
	/** Add a coordinate
	* @param c The coordinate to add
	* @param allowRepeated if set to false, repeated coordinates are collapsed
	* @return true (as by general collection contract)
	*/
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
   virtual void filter_rw(Coordinate* coord)=0;
   virtual void filter_ro(const Coordinate* coord)=0;
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
 * Constants representing the dimensions of a point, a curve and a surface.
 * Also, constants representing the dimensions of the empty geometry and
 * non-empty geometries, and a wildcard dimension meaning "any dimension".
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
    int hashCode() const;
private:
	static double distance(double x0,double y0,double x1,double y1);
	double minx;	/// the minimum x-coordinate
	double maxx;	/// the maximum x-coordinate
	double miny;	/// the minimum y-coordinate
	double maxy;	/// the maximum y-coordinate
	static const int64 serialVersionUID=5873921885273102420LL;
};

class Geometry;
class GeometryFilter;
class IntersectionMatrix;


class CGAlgorithms;
class Point;
class GeometryFactory;

/**
 *  Basic implementation of <code>Geometry</code>. <P>
 *
 *  <code>clone</code> returns a deep copy of the object.
 *
 *  <H3>Binary Predicates</H3>
 * Because it is not clear at this time
 * what semantics for spatial
 *  analysis methods involving <code>GeometryCollection</code>s would be useful,
 *  <code>GeometryCollection</code>s are not supported as arguments to binary
 *  predicates (other than <code>convexHull</code>) or the <code>relate</code>
 *  method.
 *
 *  <H3>Set-Theoretic Methods</H3>
 *
 *  The spatial analysis methods will
 *  return the most specific class possible to represent the result. If the
 *  result is homogeneous, a <code>Point</code>, <code>LineString</code>, or
 *  <code>Polygon</code> will be returned if the result contains a single
 *  element; otherwise, a <code>MultiPoint</code>, <code>MultiLineString</code>,
 *  or <code>MultiPolygon</code> will be returned. If the result is
 *  heterogeneous a <code>GeometryCollection</code> will be returned. <P>
 *
 *  Because it is not clear at this time what semantics for set-theoretic
 *  methods involving <code>GeometryCollection</code>s would be useful,
 * <code>GeometryCollections</code>
 *  are not supported as arguments to the set-theoretic methods.
 *
 *  <H4>Representation of Computed Geometries </H4>
 *
 *  The SFS states that the result
 *  of a set-theoretic method is the "point-set" result of the usual
 *  set-theoretic definition of the operation (SFS 3.2.21.1). However, there are
 *  sometimes many ways of representing a point set as a <code>Geometry</code>.
 *  <P>
 *
 *  The SFS does not specify an unambiguous representation of a given point set
 *  returned from a spatial analysis method. One goal of JTS is to make this
 *  specification precise and unambiguous. JTS will use a canonical form for
 *  <code>Geometry</code>s returned from spatial analysis methods. The canonical
 *  form is a <code>Geometry</code> which is simple and noded:
 *  <UL>
 *    <LI> Simple means that the Geometry returned will be simple according to
 *    the JTS definition of <code>isSimple</code>.
 *    <LI> Noded applies only to overlays involving <code>LineString</code>s. It
 *    means that all intersection points on <code>LineString</code>s will be
 *    present as endpoints of <code>LineString</code>s in the result.
 *  </UL>
 *  This definition implies that non-simple geometries which are arguments to
 *  spatial analysis methods must be subjected to a line-dissolve process to
 *  ensure that the results are simple.
 *
 *  <H4> Constructed Points And The Precision Model </H4>
 *
 *  The results computed by the set-theoretic methods may
 *  contain constructed points which are not present in the input <code>Geometry</code>
 *  s. These new points arise from intersections between line segments in the
 *  edges of the input <code>Geometry</code>s. In the general case it is not
 *  possible to represent constructed points exactly. This is due to the fact
 *  that the coordinates of an intersection point may contain twice as many bits
 *  of precision as the coordinates of the input line segments. In order to
 *  represent these constructed points explicitly, JTS must truncate them to fit
 *  the <code>PrecisionModel</code>. <P>
 *
 *  Unfortunately, truncating coordinates moves them slightly. Line segments
 *  which would not be coincident in the exact result may become coincident in
 *  the truncated representation. This in turn leads to "topology collapses" --
 *  situations where a computed element has a lower dimension than it would in
 *  the exact result. <P>
 *
 *  When JTS detects topology collapses during the computation of spatial
 *  analysis methods, it will throw an exception. If possible the exception will
 *  report the location of the collapse. <P>
 *
 *  #equals(Object) and #hashCode are not overridden, so that when two
 *  topologically equal Geometries are added to HashMaps and HashSets, they
 *  remain distinct. This behaviour is desired in many cases.
 *
 */
class Geometry{
friend class Unload;
public:
	void throw_exception ();
	Geometry(void);
	Geometry(const Geometry &geom);
	Geometry(const GeometryFactory *newFactory);
	/**
	* Gets the factory which contains the context in which this geometry was created.
	*
	* @return the factory for this geometry
	*/
	GeometryFactory* getFactory() const;
	/**
	* Gets the user data object for this geometry, if any.
	*
	* @return the user data object, or <code>null</code> if none set
	*/
	void* getUserData();
	/**
	* A simple scheme for applications to add their own custom data to a Geometry.
	* An example use might be to add an object representing a Coordinate Reference System.
	* <p>
	* Note that user data objects are not present in geometries created by
	* construction methods.
	*
	* @param userData an object, the semantics for which are defined by the
	* application using this Geometry
	*/
	void setUserData(void* newUserData);
	virtual string getGeometryType() const=0; //Abstract
	virtual GeometryTypeId getGeometryTypeId() const=0; //Abstract
	/**
	*  Returns the ID of the Spatial Reference System used by the <code>Geometry</code>.
	*  <P>
	*
	*  JTS supports Spatial Reference System information in the simple way
	*  defined in the SFS. A Spatial Reference System ID (SRID) is present in
	*  each <code>Geometry</code> object. <code>Geometry</code> provides basic
	*  accessor operations for this field, but no others. The SRID is represented
	*  as an integer.
	*
	*@return    the ID of the coordinate space in which the <code>Geometry</code>
	*      is defined.
	*
	*  @deprecated use {@link getUserData} instead
	*/
	virtual int getSRID() const;
		/**
	*  Sets the ID of the Spatial Reference System used by the <code>Geometry</code>.
	*  @deprecated use {@link setUserData} instead
	*/
	virtual void setSRID(int newSRID);
	virtual const PrecisionModel* getPrecisionModel() const;
	virtual const Coordinate* getCoordinate() const=0; //Abstract
	virtual CoordinateList* getCoordinates() const=0; //Abstract
	virtual int getNumPoints() const=0; //Abstract
	virtual bool isSimple() const=0; //Abstract
	/**
	*  Tests the validity of this <code>Geometry</code>.
	*  Subclasses provide their own definition of "valid".
	*
	*@return    <code>true</code> if this <code>Geometry</code> is valid
	*
	* @see IsValidOp
	*/
	virtual bool isValid() const;
	virtual bool isEmpty() const=0; //Abstract
	virtual int getDimension() const=0; //Abstract
	virtual Geometry* getBoundary() const=0; //Abstract
	virtual int getBoundaryDimension() const=0; //Abstract
	virtual Geometry* getEnvelope() const;
	virtual Envelope* getEnvelopeInternal() const;
	virtual bool disjoint(const Geometry *g) const;
	virtual bool touches(const Geometry *g) const;
	virtual bool intersects(const Geometry *g) const;
	virtual bool crosses(const Geometry *g) const;
	virtual bool within(const Geometry *g) const;
	virtual bool contains(const Geometry *g) const;
	virtual bool overlaps(const Geometry *g) const;
	virtual bool relate(const Geometry *g, string intersectionPattern) const;
	virtual IntersectionMatrix* relate(const Geometry *g) const;
	virtual bool equals(const Geometry *g) const;
	virtual string toString() const;
	virtual string toText() const;
	virtual Geometry* buffer(double distance) const;
	virtual Geometry* buffer(double distance,int quadrantSegments) const;
	virtual Geometry* convexHull() const;
	virtual Geometry* intersection(const Geometry *other) const;
	virtual Geometry* Union(const Geometry *other) const; // throw(IllegalArgumentException *, TopologyException *);
	virtual Geometry* difference(const Geometry *other) const;
	virtual Geometry* symDifference(const Geometry *other) const;
	virtual bool equalsExact(const Geometry *other, double tolerance) const=0; //Abstract
	virtual void apply_rw(CoordinateFilter *filter)=0; //Abstract
	virtual void apply_ro(CoordinateFilter *filter) const=0; //Abstract
	virtual void apply_rw(GeometryFilter *filter);
	virtual void apply_ro(GeometryFilter *filter) const;
	virtual void apply_rw(GeometryComponentFilter *filter);
	virtual void apply_ro(GeometryComponentFilter *filter) const;
	virtual Geometry* clone() const=0;
	virtual void normalize()=0; //Abstract
	virtual int compareTo(const Geometry *geom) const;
	virtual double distance(const Geometry *g) const;
	virtual double getArea() const;
	virtual double getLength() const;
	virtual ~Geometry();
	virtual bool isWithinDistance(const Geometry *geom,double cDistance);
	virtual Point* getCentroid() const;
	virtual Point* getInteriorPoint();
	virtual void geometryChanged();
	void geometryChangedAction();
protected:
	Envelope* envelope;
	static bool hasNonEmptyElements(const vector<Geometry *>* geometries);
	static bool hasNullElements(const CoordinateList* list);
	static bool hasNullElements(const vector<Geometry *>* lrs);
//	static void reversePointOrder(CoordinateList* coordinates);
//	static Coordinate& minCoordinate(CoordinateList* coordinates);
//	static void scroll(CoordinateList* coordinates,Coordinate* firstCoordinate);
//	static int indexOf(Coordinate* coordinate,CoordinateList* coordinates);
	virtual bool isEquivalentClass(const Geometry *other) const;
	static void checkNotGeometryCollection(const Geometry *g); // throw(IllegalArgumentException *);
	//virtual void checkEqualSRID(Geometry *other);
	//virtual void checkEqualPrecisionModel(Geometry *other);
	virtual Envelope* computeEnvelopeInternal() const=0; //Abstract
	virtual int compareToSameClass(const Geometry *geom) const=0; //Abstract
	int compare(vector<Coordinate> a, vector<Coordinate> b) const;
	int compare(vector<Geometry *> a, vector<Geometry *> b) const;
	bool equal(const Coordinate& a, const Coordinate& b,double tolerance) const;
	int SRID;
	/**
	* The JTS algorithms assume that Geometry#getCoordinate and #getCoordinates
	* are fast, which may not be the case if the CoordinateSequence is not a
	* BasicCoordinateSequence (e.g. if it were implemented using separate arrays
	* for the x- and y-values), in which case frequent construction of Coordinates
	* takes up much space and time. To solve this performance problem,
	* #toInternalGeometry converts the Geometry to a BasicCoordinateSequence
	* implementation before sending it to the JTS algorithms.
	*/
	Geometry* toInternalGeometry(const Geometry *g) const;
	Geometry* fromInternalGeometry(const Geometry *g) const;
private:
	virtual int getClassSortIndex() const;
	static GeometryComponentFilter geometryChangedFilter;
    static const int64 serialVersionUID = 8763622679187376702LL;
	GeometryFactory *factory;
	static const GeometryFactory* INTERNAL_GEOMETRY_FACTORY;
	void* userData;
	Point* createPointFromInternalCoord(const Coordinate* coord,const Geometry *exemplar) const;
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
 * Represents a line segment defined by two {@link Coordinate}s.
 * Provides methods to compute various geometric properties
 * and relationships of line segments.
 * <p>
 * This class is designed to be easily mutable (to the extent of
 * having its contained points public).
 * This supports a common pattern of reusing a single LineSegment
 * object as a way of computing segment properties on the
 * segments defined by arrays or lists of {@link Coordinate}s.
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
	/**
	* Tests whether the segment is horizontal.
	*
	* @return <code>true</code> if the segment is horizontal
	*/
	virtual bool isHorizontal() const;
	/**
	* Tests whether the segment is vertical.
	*
	* @return <code>true</code> if the segment is vertical
	*/
	virtual bool isVertical() const;
	/**
	* Determines the orientation of a LineSegment relative to this segment.
	* The concept of orientation is specified as follows:
	* Given two line segments A and L,
	* <ul
	* <li>A is to the left of a segment L if A lies wholly in the
	* closed half-plane lying to the left of L
	* <li>A is to the right of a segment L if A lies wholly in the
	* closed half-plane lying to the right of L
	* <li>otherwise, A has indeterminate orientation relative to L. This
	* happens if A is collinear with L or if A crosses the line determined by L.
	* </ul>
	*
	* @param seg the LineSegment to compare
	*
	* @return 1 if <code>seg</code> is to the left of this segment
	* @return -1 if <code>seg</code> is to the right of this segment
	* @return 0 if <code>seg</code> has indeterminate orientation relative to this segment
	*/
	virtual int orientationIndex(LineSegment *seg) const;
	virtual void reverse();
	virtual void normalize();
	virtual double angle() const;
	virtual double distance(const LineSegment ls) const;
	/**
	* Computes the distance between this line segment and a point.
	*/
	virtual double distance(const Coordinate& p) const;
	/**
	* Computes the perpendicular distance between the (infinite) line defined
	* by this line segment and a point.
	*/
	virtual double distancePerpendicular(const Coordinate& p) const;
	virtual double projectionFactor(const Coordinate& p) const;
	virtual Coordinate* project(const Coordinate& p) const;
	virtual LineSegment* project(const LineSegment *seg) const;
	virtual Coordinate* closestPoint(const Coordinate& p) const;
	virtual int compareTo(const LineSegment other) const;
	virtual bool equalsTopo(const LineSegment other) const;
	/**
	* Computes the closest points on two line segments.
	* @param p the point to find the closest point to
	* @return a pair of Coordinates which are the closest points on the line segments
	* The returned CoordianteList must be deleted by caller
	*/
	virtual CoordinateList* closestPoints(const LineSegment *line);
	/**
	* Computes an intersection point between two segments, if there is one.
	* There may be 0, 1 or many intersection points between two segments.
	* If there are 0, null is returned. If there is 1 or more, a single one
	* is returned (chosen at the discretion of the algorithm).  If
	* more information is required about the details of the intersection,
	* the {@link RobustLineIntersector} class should be used.
	*
	* @param line
	* @return an intersection point, or <code>null</code> if there is none
	*/
	Coordinate* intersection(const LineSegment *line) const;
	virtual string toString() const;
private:
  static const int64 serialVersionUID=3252005833466256227LL;

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


//Operators
bool operator==(const Coordinate& a, const Coordinate& b);
bool operator==(const Envelope a, const Envelope b);
bool operator==(const PrecisionModel a, const PrecisionModel b);
bool operator==(const LineSegment a, const LineSegment b);

bool lessThen(Coordinate& a,Coordinate& b);
bool greaterThen(Geometry *first, Geometry *second);

class GeometryCollection : public Geometry{
public:
//	GeometryCollection(void);
	GeometryCollection(const GeometryCollection &gc);
	GeometryCollection(const vector<Geometry *> *newGeometries,PrecisionModel* pm, int SRID);
	/**
	* @param geometries
	*            the <code>Geometry</code>s for this <code>GeometryCollection</code>,
	*            or <code>null</code> or an empty array to create the empty
	*            geometry. Elements may be empty <code>Geometry</code>s,
	*            but not <code>null</code>s.
	*/
	GeometryCollection(const vector<Geometry *> *newGeometries, const GeometryFactory *newFactory);
	virtual Geometry *clone() const;
	virtual ~GeometryCollection();
	/**
	* Collects all coordinates of all subgeometries into a CoordinateList.
	* 
	* Note that while changes to the coordinate objects themselves
	* may modify the Geometries in place, the returned CoordinateList as such 
	* is only a temporary container which is not synchronized back.
	* 
	* @return the collected coordinates
	*
	*/
	virtual CoordinateList* getCoordinates() const;
	virtual bool isEmpty() const;
	virtual int getDimension() const;
	virtual int getBoundaryDimension() const;
	virtual int getNumGeometries() const;
	virtual const Geometry* getGeometryN(int n) const;
	virtual int getNumPoints() const;
	virtual string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const {
		return GEOS_GEOMETRYCOLLECTION;
	}
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
//	virtual Point* getCentroid() const;
protected:
	vector<Geometry *>* geometries;
	virtual Envelope* computeEnvelopeInternal() const;
	virtual int compareToSameClass(const Geometry *gc) const;
private:
	static const int64 serialVersionUID = -5694727726395021467LL;
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

/**
 *  Basic implementation of <code>Point</code>.
 *
 */
class Point : public Geometry{
public:
	/**
	*  Constructs a <code>Point</code> with the given coordinate.
	*
	*@param  coordinate      the coordinate on which to base this <code>Point</code>
	*      , or <code>null</code> to create the empty geometry.
	*@param  precisionModel  the specification of the grid of allowable points
	*      for this <code>Point</code>
	*@param  SRID            the ID of the Spatial Reference System used by this
	*      <code>Point</code>
	* @deprecated Use GeometryFactory instead
	*/
	Point(const Coordinate& c, const PrecisionModel* pm, int SRID);
	Point(const CoordinateList *newCoordinates, const GeometryFactory *newFactory);
	Point(const Point &p); 
	virtual ~Point();
	Geometry *clone() const;
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
	virtual GeometryTypeId getGeometryTypeId() const {
		return GEOS_POINT;
	}
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
	Envelope* computeEnvelopeInternal() const;
	int compareToSameClass(const Geometry *p) const;
private:
	/**
	*  The <code>Coordinate</code> wrapped by this <code>Point</code>.
	*/
	CoordinateList *coordinates;
	static const int64 serialVersionUID = 4902022702746614570LL;
};

/**
 *  Basic implementation of <code>LineString</code>.
 *
 */
class LineString: public Geometry {
public:
//	LineString();
	LineString(const LineString &ls);
	/**
	*  Constructs a <code>LineString</code> with the given points.
	*
	*@param  points          the points of the linestring, or <code>null</code>
	*      to create the empty geometry. This array must not contain <code>null</code>
	*      elements. Consecutive points may not be equal.
	*@param  precisionModel  the specification of the grid of allowable points
	*      for this <code>LineString</code>
	*@param  SRID            the ID of the Spatial Reference System used by this
	*      <code>LineString</code>
	* @deprecated Use GeometryFactory instead 
	*/  
	LineString(const CoordinateList *pts, const PrecisionModel *pm, int SRID);
	/**
	*@param  points          the points of the linestring, or <code>null</code>
	*      to create the empty geometry. Consecutive points may not be equal.
	*/  
	LineString(const CoordinateList *pts, const GeometryFactory *newFactory);
	virtual ~LineString();
	virtual Geometry *clone() const;
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
	virtual GeometryTypeId getGeometryTypeId() const {
		return GEOS_LINESTRING;
	}
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
	virtual Envelope* computeEnvelopeInternal() const;
	virtual bool isEquivalentClass(const Geometry *other) const;
	CoordinateList* points;
private:
	static const int64 serialVersionUID = 3110669828065365560LL;
};

/**
 *  Basic implementation of <code>LinearRing</code>.
 * The first and last point in the coordinate sequence must be equal.
 * Either orientation of the ring is allowed.
 * A valid ring must not self-intersect.
 *
 */
class LinearRing : public LineString{
public:
//	LinearRing();
	LinearRing(const LinearRing &lr);
	/**
	*  Constructs a <code>LinearRing</code> with the given points.
	*
	*@param  points          points forming a closed and simple linestring, or
	*      <code>null</code> or an empty array to create the empty geometry.
	*      This array must not contain <code>null</code> elements.
	*
	*@param  precisionModel  the specification of the grid of allowable points
	*      for this <code>LinearRing</code>
	*@param  SRID            the ID of the Spatial Reference System used by this
	*      <code>LinearRing</code>
	* @deprecated Use GeometryFactory instead
	*/
	LinearRing(const CoordinateList* points, const PrecisionModel* pm, int SRID);
	/**
	*  Constructs a <code>LinearRing</code> with the given points.
	*
	*@param  points          points forming a closed and simple linestring, or
	*      <code>null</code> or an empty array to create the empty geometry.
	*      This array must not contain <code>null</code> elements.
	*
	*/
	LinearRing(const CoordinateList* points, const GeometryFactory *newFactory);
	virtual ~LinearRing();
	bool isSimple() const;
	string getGeometryType() const;
	bool isClosed() const;
	void setPoints(CoordinateList* cl);
private:
	static const int64 serialVersionUID = -4261142084085851829LL;
	void validateConstruction();
};

/**
 * Represents a linear polygon, which may include holes.
 * The shell and holes of the polygon are represented by {@link LinearRing}s.
 * In a valid polygon, holes may touch the shell or other holes at a single point.
 * However, no sequence of touching holes may split the polygon into two pieces.
 * The orientation of the rings in the polygon does not matter.
 * <p>
 *  The shell and holes must conform to the assertions specified in the <A
 *  HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
 *  Specification for SQL</A> .
 *
 */
class Polygon: public Geometry{
public:
	Polygon();
	Polygon(const Polygon &p);
	virtual ~Polygon();
	/**
	*  Constructs a <code>Polygon</code> with the given exterior boundary.
	*
	*@param  shell           the outer boundary of the new <code>Polygon</code>,
	*      or <code>null</code> or an empty <code>LinearRing</code> if the empty
	*      geometry is to be created.
	*@param  precisionModel  the specification of the grid of allowable points
	*      for this <code>Polygon</code>
	*@param  SRID            the ID of the Spatial Reference System used by this
	*      <code>Polygon</code>
	* @deprecated Use GeometryFactory instead
	*/
	Polygon(LinearRing *newShell, PrecisionModel* precisionModel, int SRID);
	/**
	*  Constructs a <code>Polygon</code> with the given exterior boundary and
	*  interior boundaries.
	*
	*@param  shell           the outer boundary of the new <code>Polygon</code>,
	*      or <code>null</code> or an empty <code>LinearRing</code> if the empty
	*      geometry is to be created.
	*@param  holes           the inner boundaries of the new <code>Polygon</code>
	*      , or <code>null</code> or empty <code>LinearRing</code>s if the empty
	*      geometry is to be created.
	*@param  precisionModel  the specification of the grid of allowable points
	*      for this <code>Polygon</code>
	*@param  SRID            the ID of the Spatial Reference System used by this
	*      <code>Polygon</code>
	* @deprecated Use GeometryFactory instead
	*/
	Polygon(LinearRing *newShell, vector<Geometry *> *newHoles, PrecisionModel* precisionModel, int SRID);
	/**
	*  Constructs a <code>Polygon</code> with the given exterior boundary and
	*  interior boundaries.
	*
	*@param  shell           the outer boundary of the new <code>Polygon</code>,
	*      or <code>null</code> or an empty <code>LinearRing</code> if the empty
	*      geometry is to be created.
	*@param  holes           the inner boundaries of the new <code>Polygon</code>
	*      , or <code>null</code> or empty <code>LinearRing</code>s if the empty
	*      geometry is to be created.
	*/
	Polygon(LinearRing *newShell, vector<Geometry *> *newHoles, const GeometryFactory *newFactory);
	virtual Geometry *clone() const;
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
	virtual GeometryTypeId getGeometryTypeId() const {
		return GEOS_POLYGON;
	}
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
	static const int64 serialVersionUID = -3494792200821764533LL;
};

/**
 *  Models a collection of <code>Point</code>s.
 *
 */
class MultiPoint: public GeometryCollection{
public:
//	MultiPoint();
	/**
	*  Constructs a <code>MultiPoint</code>.
	*
	*@param  points          the <code>Point</code>s for this <code>MultiPoint</code>
	*      , or <code>null</code> or an empty array to create the empty geometry.
	*      Elements may be empty <code>Point</code>s, but not <code>null</code>s.
	*@param  precisionModel  the specification of the grid of allowable points
	*      for this <code>MultiPoint</code>
	*@param  SRID            the ID of the Spatial Reference System used by this
	*      <code>MultiPoint</code>
	* @deprecated Use GeometryFactory instead
	*/
	MultiPoint(const vector<Geometry *> *points,PrecisionModel* pm, int SRID);
	/**
	*@param  points          the <code>Point</code>s for this <code>MultiPoint</code>
	*      , or <code>null</code> or an empty array to create the empty geometry.
	*      Elements may be empty <code>Point</code>s, but not <code>null</code>s.
	*/
	MultiPoint(const vector<Geometry *> *points, const GeometryFactory *newFactory);
	virtual ~MultiPoint();
	int getDimension() const;
	int getBoundaryDimension() const;
	string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const {
		return GEOS_MULTIPOINT;
	}
	bool isValid() const;
	bool isClosed() const;
	bool isSimple() const;
	Geometry* getBoundary() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
protected:
	const Coordinate* getCoordinate(int n) const;
private:
	static const int64 serialVersionUID = -8048474874175355449LL;
};

/**
 *  Basic implementation of <code>MultiLineString</code>.
 *
 */
class MultiLineString: public GeometryCollection{
public:
//	MultiLineString();
	/**
	*  Constructs a <code>MultiLineString</code>.
	*
	*@param  lineStrings     the <code>LineString</code>s for this <code>MultiLineString</code>
	*      , or <code>null</code> or an empty array to create the empty geometry.
	*      Elements may be empty <code>LineString</code>s, but not <code>null</code>
	*      s.
	*@param  precisionModel  the specification of the grid of allowable points
	*      for this <code>MultiLineString</code>
	*@param  SRID            the ID of the Spatial Reference System used by this
	*      <code>MultiLineString</code>
	* @deprecated Use GeometryFactory instead
	*/
	MultiLineString(const vector<Geometry *> *lineStrings, PrecisionModel* precisionModel, int SRID);
	MultiLineString(const vector<Geometry *> *lineStrings, const GeometryFactory *newFactory);
	virtual ~MultiLineString();
	int getDimension() const;
	int getBoundaryDimension() const;
	string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const {
		return GEOS_MULTILINESTRING;
	}
	bool isClosed() const;
	bool isSimple() const;
	Geometry* getBoundary() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
private:
	static const int64 serialVersionUID = 8166665132445433741LL;
};

/**
 *  Basic implementation of <code>MultiPolygon</code>.
 *
 */
class MultiPolygon: public GeometryCollection {
public:
	//MultiPolygon();
	/**
	*  Constructs a <code>MultiPolygon</code>.
	*
	*@param  polygons        the <code>Polygon</code>s for this <code>MultiPolygon</code>
	*      , or <code>null</code> or an empty array to create the empty geometry.
	*      Elements may be empty <code>Polygon</code>s, but not <code>null</code>
	*      s. The polygons must conform to the assertions specified in the <A
	*      HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
	*      Specification for SQL</A> .
	*@param  precisionModel  the specification of the grid of allowable points
	*      for this <code>MultiPolygon</code>
	*@param  SRID            the ID of the Spatial Reference System used by this
	*      <code>MultiPolygon</code>
	* @deprecated Use GeometryFactory instead
	*/
	MultiPolygon(const vector<Geometry *> *polygons, PrecisionModel* precisionModel, int SRID);
	/**
	* @param polygons
	*            the <code>Polygon</code>s for this <code>MultiPolygon</code>,
	*            or <code>null</code> or an empty array to create the empty
	*            geometry. Elements may be empty <code>Polygon</code>s, but
	*            not <code>null</code>s. The polygons must conform to the
	*            assertions specified in the <A
	*            HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple
	*            Features Specification for SQL</A>.
	*/
	MultiPolygon(const vector<Geometry *> *polygons, const GeometryFactory *newFactory);
	virtual ~MultiPolygon();
	int getDimension() const;
	int getBoundaryDimension() const;
	string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const {
		return GEOS_MULTIPOLYGON;
	}
	bool isSimple() const;
	Geometry* getBoundary() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
private:
	static const int64 serialVersionUID = -551033529766975875LL;
};

/**
 * Supplies a set of utility methods for building Geometry objects from lists
 * of Coordinates.
 *
 */
class GeometryFactory {
public:
	/**
	* Constructs a GeometryFactory that generates Geometries having a floating
	* PrecisionModel and a spatial-reference ID of 0.
	*/
	GeometryFactory();
	/**
	* Constructs a GeometryFactory that generates Geometries having the given
	* PrecisionModel, spatial-reference ID, and CoordinateSequence implementation.
	*/
	GeometryFactory(const PrecisionModel *pm, int newSRID,CoordinateListFactory *nCoordinateListFactory);
	/**
	* Constructs a GeometryFactory that generates Geometries having the given
	* CoordinateList implementation, a double-precision floating PrecisionModel and a
	* spatial-reference ID of 0.
	*/
	GeometryFactory(CoordinateListFactory *nCoordinateListFactory);
	/**
	* Constructs a GeometryFactory that generates Geometries having the given
	* {@link PrecisionModel} and the default CoordinateSequence
	* implementation.
	*
	* @param precisionModel the PrecisionModel to use
	*/
	GeometryFactory(const PrecisionModel *pm);
	/**
	* Constructs a GeometryFactory that generates Geometries having the given
	* {@link PrecisionModel} and spatial-reference ID, and the default CoordinateSequence
	* implementation.
	*
	* @param precisionModel the PrecisionModel to use
	* @param SRID the SRID to use
	*/
	GeometryFactory(const PrecisionModel* pm, int newSRID);
	/**
	* Copy constructor
	*
	* @param gf the GeometryFactory to clone from
	*/
	GeometryFactory(const GeometryFactory &gf);
	virtual ~GeometryFactory();

//Skipped a lot of list to array convertors

	Point* createPointFromInternalCoord(const Coordinate* coord, const Geometry *exemplar) const;
	Geometry* toGeometry(Envelope* envelope) const;
	/**
	* Returns the PrecisionModel that Geometries created by this factory
	* will be associated with.
	*/
	const PrecisionModel* getPrecisionModel() const;
	Point* createPoint(const Coordinate& coordinate) const;
	/**
	* Creates a Point using the given CoordinateSequence; a null or empty
	* CoordinateSequence will create an empty Point.
	*/
	Point* createPoint(const CoordinateList *coordinates) const;
	/**
	* Creates a MultiLineString using the given LineStrings; a null or empty
	* array will create an empty MultiLineString.
	* @param lineStrings LineStrings, each of which may be empty but not null
	*/
	MultiLineString* createMultiLineString(vector<Geometry *> *lineStrings) const;
	/**
	* Creates a GeometryCollection using the given Geometries; a null or empty
	* array will create an empty GeometryCollection.
	* @param geometries Geometries, each of which may be empty but not null
	*/
	GeometryCollection* createGeometryCollection(vector<Geometry *> *geometries) const;
	/**
	* Creates a MultiPolygon using the given Polygons; a null or empty array
	* will create an empty Polygon. The polygons must conform to the
	* assertions specified in the <A
	* HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
	* Specification for SQL</A>.
	*
	* @param polygons
	*            Polygons, each of which may be empty but not null
	*/
	MultiPolygon* createMultiPolygon(vector<Geometry *> *polygons) const;
	/**
	* Creates a LinearRing using the given CoordinateSequence; a null or empty CoordinateSequence will
	* create an empty LinearRing. The points must form a closed and simple
	* linestring. Consecutive points must not be equal.
	* @param coordinates a CoordinateSequence possibly empty, or null
	*/
	LinearRing* createLinearRing(CoordinateList* coordinates) const;
	/**
	* Creates a MultiPoint using the given Points; a null or empty array will
	* create an empty MultiPoint.
	* @param coordinates an array without null elements, or an empty array, or null
	*/
	MultiPoint* createMultiPoint(vector<Geometry *> *point) const;
	/**
	* Creates a MultiPoint using the given CoordinateSequence; a null or empty CoordinateSequence will
	* create an empty MultiPoint.
	* @param coordinates a CoordinateSequence possibly empty, or null
	*/
	MultiPoint* createMultiPoint(const CoordinateList* coordinates) const;
	/**
	* Constructs a <code>Polygon</code> with the given exterior boundary and
	* interior boundaries.
	*
	* @param shell
	*            the outer boundary of the new <code>Polygon</code>, or
	*            <code>null</code> or an empty <code>LinearRing</code> if
	*            the empty geometry is to be created.
	* @param holes
	*            the inner boundaries of the new <code>Polygon</code>, or
	*            <code>null</code> or empty <code>LinearRing</code> s if
	*            the empty geometry is to be created.
	*/
	Polygon* createPolygon(LinearRing *shell, vector<Geometry *> *holes) const;
	/**
	* Creates a LineString using the given Coordinates; a null or empty array will
	* create an empty LineString. Consecutive points must not be equal.
	* @param coordinates an array without null elements, or an empty array, or null
	*/
	LineString* createLineString(const CoordinateList* coordinates) const;
	/**
	*  Build an appropriate <code>Geometry</code>, <code>MultiGeometry</code>, or
	*  <code>GeometryCollection</code> to contain the <code>Geometry</code>s in
	*  it.
	* For example:<br>
	*
	*  <ul>
	*    <li> If <code>geomList</code> contains a single <code>Polygon</code>,
	*    the <code>Polygon</code> is returned.
	*    <li> If <code>geomList</code> contains several <code>Polygon</code>s, a
	*    <code>MultiPolygon</code> is returned.
	*    <li> If <code>geomList</code> contains some <code>Polygon</code>s and
	*    some <code>LineString</code>s, a <code>GeometryCollection</code> is
	*    returned.
	*    <li> If <code>geomList</code> is empty, an empty <code>GeometryCollection</code>
	*    is returned
	*  </ul>
	*
	* Note that this method does not "flatten" Geometries in the input, and hence if
	* any MultiGeometries are contained in the input a GeometryCollection containing
	* them will be returned.
	*
	*@param  geomList  the <code>Geometry</code>s to combine
	*@return           a <code>Geometry</code> of the "smallest", "most
	*      type-specific" class that can contain the elements of <code>geomList</code>
	*      .
	*/
	Geometry* buildGeometry(vector<Geometry *> *geoms) const;
	int getSRID() const {return SRID;};
	CoordinateListFactory* getCoordinateListFactory() const {return coordinateListFactory;};
	/**
	* @return a clone of g based on a CoordinateSequence created by this
	* GeometryFactory's CoordinateSequenceFactory
	*/
	Geometry* createGeometry(const Geometry *g) const;
private:
	const PrecisionModel* precisionModel;
	int SRID;
	static const int64 serialVersionUID = -6820524753094095635LL;
	CoordinateListFactory *coordinateListFactory;

};

/**
 * Represents a planar triangle, and provides methods for calculating various
 * properties of triangles.
 *
 */
class Triangle {
public:
	Coordinate p0,p1,p2;
	Triangle(const Coordinate& nP0,const Coordinate& nP1,const Coordinate& nP2);
	/**
	* The inCentre of a triangle is the point which is equidistant
	* from the sides of the triangle.  This is also the point at which the bisectors
	* of the angles meet.
	*
	* @return the point which is the inCentre of the triangle
	*/
	Coordinate* inCentre();
};
}
#endif
