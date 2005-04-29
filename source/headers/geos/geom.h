/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
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
//#include <math.h>
#include <cmath>
#include <geos/platform.h>

using namespace std;

/**
 * \brief Basic namespace for all GEOS functionalities.
 */
namespace geos {

/// An unsigned char
typedef unsigned char byte;

/// Return current GEOS version
string geosversion();

/**
 * \brief
 * Return the version of JTS this GEOS
 * release has been ported from.
 */
string jtsport();

/// Geometry types
enum GeometryTypeId {
	/// a point
	GEOS_POINT,
	/// a linestring
	GEOS_LINESTRING,
	/// a linear ring (linestring with 1st point == last point)
	GEOS_LINEARRING,
	/// a polygon
	GEOS_POLYGON,
	/// a collection of points
	GEOS_MULTIPOINT,
	/// a collection of linestrings
	GEOS_MULTILINESTRING,
	/// a collection of polygons
	GEOS_MULTIPOLYGON,
	/// a collection of heterogeneus geometries
	GEOS_GEOMETRYCOLLECTION
};

class Coordinate;

/**
 * \class PrecisionModel geom.h geos.h
 *
 * \brief Specifies the precision model of the Coordinate in a Geometry.
 *
 * In other words, specifies the grid of allowable
 * points for all <code>Geometry</code>s.
 * 
 * The makePrecise method allows rounding a coordinate to
 * a "precise" value; that is, one whose
 * precision is known exactly.
 *
 * Coordinates are assumed to be precise in geometries.
 * That is, the coordinates are assumed to be rounded to the
 * precision model given for the geometry.
 * JTS input routines automatically round coordinates to the precision model
 * before creating Geometries.
 * All internal operations
 * assume that coordinates are rounded to the precision model.
 * Constructive methods (such as boolean operations) always round computed
 * coordinates to the appropriate precision model.
 *
 * Currently three types of precision model are supported:
 * - FLOATING - represents full double precision floating point.
 *   This is the default precision model used in JTS
 * - FLOATING_SINGLE - represents single precision floating point.
 * - FIXED - represents a model with a fixed number of decimal places.
 *   A Fixed Precision Model is specified by a scale factor.
 *   The scale factor specifies the grid which numbers are rounded to.
 *   Input coordinates are mapped to fixed coordinates according to the
 *   following equations:
 *   - jtsPt.x = round( (inputPt.x * scale ) / scale
 *   - jtsPt.y = round( (inputPt.y * scale ) / scale
 *
 * Coordinates are represented internally as Java double-precision values.
 * Since Java uses the IEEE-394 floating point standard, this
 * provides 53 bits of precision. (Thus the maximum precisely representable
 * integer is 9,007,199,254,740,992).
 *
 * JTS methods currently do not handle inputs with different precision models.
 */
class PrecisionModel {
friend class Unload;
public:
	/// The types of Precision Model which GEOS supports.
	/*
	* This class is only for use to support the "enums"
	* for the types of precision model.
	*/
	typedef enum {

		/**
		* Fixed Precision indicates that coordinates have a fixed
		* number of decimal places.
		* The number of decimal places is determined by the log10
		* of the scale factor.
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
	
	/// Creates a PrecisionModel with a default precision of FLOATING.
	PrecisionModel(void);

	/// Creates a PrecisionModel specifying an explicit precision model type.
	/**
	* If the model type is FIXED the scale factor will default to 1.
	*
	* @param nModelType the type of the precision model
	*/
	PrecisionModel(Type nModelType);

	/*
	 * Creates a <code>PrecisionModel</code> with Fixed precision.
	 *
	 * Fixed-precision coordinates are represented as precise internal
	 * coordinates, which are rounded to the grid defined by the
	 * scale factor.
	 *
	 * @param  scale
	 *	amount by which to multiply a coordinate after subtracting
	 *	the offset, to obtain a precise coordinate
	 * @param  offsetX  not used.
	 * @param  offsetY  not used.
	 *
	 * @deprecated offsets are no longer supported, since internal
	 * representation is rounded floating point
	 */
	PrecisionModel(double newScale, double newOffsetX, double newOffsetY);

	/**
	 * \brief 
	 * Creates a PrecisionModel with Fixed precision.
	 *
	 * Fixed-precision coordinates are represented as precise
	 * internal coordinates which are rounded to the grid defined
	 * by the scale factor.
	 *
	 * @param newScale amount by which to multiply a coordinate
	 * after subtracting the offset, to obtain a precise coordinate
	 */
	PrecisionModel(double newScale);

	// copy constructor
	PrecisionModel(const PrecisionModel &pm);

	/// destructor
	virtual ~PrecisionModel(void);


	/// The maximum precise value representable in a double.
	/**
	 * Since IEE754 double-precision numbers allow 53 bits of mantissa,
	 * the value is equal to 2^53 - 1.
	 * This provides <i>almost</i> 16 decimal digits of precision.
	 */
	static const double maximumPreciseValue;

	/// Rounds a numeric value to the PrecisionModel grid.
	double makePrecise(double val) const;

	/// Rounds the given Coordinate to the PrecisionModel grid.
	void makePrecise(Coordinate *coord) const;

	/// Tests whether the precision model supports floating point
	/**
	* @return <code>true</code> if the precision model supports
	* floating point
	*/
	bool isFloating() const;

	/// Returns the maximum number of significant digits provided by this precision model.
	/**
	* Intended for use by routines which need to print out precise values.
	*
	* @return the maximum number of decimal places provided by this precision model
	*/
	int getMaximumSignificantDigits() const;

	/// Gets the type of this PrecisionModel
	/**
	* @return the type of this PrecisionModel
	*/
	Type getType() const;

	/// Returns the multiplying factor used to obtain a precise coordinate.
	double getScale() const;

	/*
	* Returns the x-offset used to obtain a precise coordinate.
	*
	* @return the amount by which to subtract the x-coordinate before
	*         multiplying by the scale
	* @deprecated Offsets are no longer used
	*/
	double getOffsetX() const;

	/*
	* Returns the y-offset used to obtain a precise coordinate.
	*
	* @return the amount by which to subtract the y-coordinate before
	*         multiplying by the scale
	* @deprecated Offsets are no longer used
	*/
	double getOffsetY() const;

	/*
	 *  Sets <code>internal</code> to the precise representation of <code>external</code>.
	 *
	 * @param external the original coordinate
	 * @param internal the coordinate whose values will be changed to the
	 *                 precise representation of <code>external</code>
	 * @deprecated use makePrecise instead
	 */
	void toInternal(const Coordinate& external, Coordinate* internal) const;

	/*
	*  Returns the precise representation of <code>external</code>.
	*
	*@param  external  the original coordinate
	*@return
	*	the coordinate whose values will be changed to the precise
	*	representation of <code>external</code>
	* @deprecated use makePrecise instead
	*/
	Coordinate* toInternal(const Coordinate& external) const;

	/*
	*  Returns the external representation of <code>internal</code>.
	*
	*@param  internal  the original coordinate
	*@return           the coordinate whose values will be changed to the
	*      external representation of <code>internal</code>
	* @deprecated no longer needed, since internal representation is same as external representation
	*/
	Coordinate* toExternal(const Coordinate& internal) const;

	/*
	*  Sets <code>external</code> to the external representation of
	*  <code>internal</code>.
	*
	* @param  internal  the original coordinate
	* @param  external
	*	the coordinate whose values will be changed to the
	*	external representation of <code>internal</code>
	* @deprecated no longer needed, since internal representation is same as external representation
	*/
	void toExternal(const Coordinate& internal, Coordinate* external) const;

	string toString() const;

	/// Compares this PrecisionModel object with the specified object for order.
	/**
	* A PrecisionModel is greater than another if it provides greater precision.
	* The comparison is based on the value returned by the
	* getMaximumSignificantDigits method.
	* This comparison is not strictly accurate when comparing floating
	* precision models to fixed models;
	* however, it is correct when both models are either floating or fixed.
	*
	* @param other the PrecisionModel with which this PrecisionModel
	*      is being compared
	* @return a negative integer, zero, or a positive integer as this
	*      PrecisionModel is less than, equal to, or greater than the
	*      specified PrecisionModel.
	*/
	int compareTo(const PrecisionModel* other) const;

private:
	void setScale(double newScale);
	Type modelType;
	double scale;
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = 7777263578777803835I64;
#else        
	static const int64 serialVersionUID = 7777263578777803835LL;
#endif        
};

/**
 * \class Coordinate geom.h geos.h
 *
 * \brief
 * Coordinate is the lightweight class used to store coordinates.
 *
 * It is distinct from Point, which is a subclass of Geometry.
 * Unlike objects of type Point (which contain additional
 * information such as an envelope, a precision model, and spatial
 * reference system information), a Coordinate only contains
 * ordinate values and accessor methods. 
 *
 * Coordinate objects are two-dimensional points, with an additional
 * z-ordinate. JTS does not support any operations on the z-ordinate except
 * the basic accessor functions.
 *
 * Constructed coordinates will have a z-ordinate of DoubleNotANumber.
 * The standard comparison functions will ignore the z-ordinate.
 *
 */
class Coordinate {
public:
	inline void setNull(void);
	inline static Coordinate& getNull(void);
	//virtual ~Coordinate(){};
	inline Coordinate();
	inline Coordinate(double xNew, double yNew, double zNew);
	inline Coordinate(const Coordinate& c);
	inline Coordinate(double xNew, double yNew);
	inline void setCoordinate(const Coordinate& other);
	inline bool equals2D(const Coordinate& other) const;
	inline int compareTo(const Coordinate& other) const;
	inline bool equals3D(const Coordinate& other) const;
	string toString() const;
	inline void makePrecise(const PrecisionModel *pm);
	inline double distance(const Coordinate& p) const;
	static Coordinate nullCoord;

	inline int hashCode() const;
	inline static int hashCode(double);


	/// x-coordinate
	double x;
	/// y-coordinate
	double y;
	/// z-coordinate
	double z;

private:
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID=6683108902428366910I64;
#else
	static const int64 serialVersionUID=6683108902428366910LL;
#endif

};

// INLINE FUNCTION FOR Coordinate CLASS

inline void
Coordinate::setNull()
{
		x=DoubleNotANumber;
		y=DoubleNotANumber;
		z=DoubleNotANumber;
}

inline Coordinate&
Coordinate::getNull()
{
	return nullCoord;
}

inline
Coordinate::Coordinate()
{
	x=0.0;
	y=0.0;
	z=DoubleNotANumber;
}

inline
Coordinate::Coordinate(double xNew, double yNew, double zNew)
{
	x=xNew;
	y=yNew;
	z=zNew;
}

inline
Coordinate::Coordinate(const Coordinate& c)
{
	x=c.x;
	y=c.y;
	z=c.z;
}

inline
Coordinate::Coordinate(double xNew, double yNew)
{
	x=xNew;
	y=yNew;
	z=DoubleNotANumber;
}

inline void
Coordinate::setCoordinate(const Coordinate& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
}

inline bool
Coordinate::equals2D(const Coordinate& other) const
{
	if (x != other.x) {
		return false;
	}
	if (y != other.y) {
		return false;
	}
	return true;
}

inline int
Coordinate::compareTo(const Coordinate& other) const
{
	if (x < other.x) return -1;
	if (x > other.x) return 1;
	if (y < other.y) return -1;
	if (y > other.y) return 1;
	return 0;
}

inline bool
Coordinate::equals3D(const Coordinate& other) const
{
	return (x == other.x) && ( y == other.y) && 
		((z == other.z)||(ISNAN(z) && ISNAN(other.z)));
}

inline void
Coordinate::makePrecise(const PrecisionModel *precisionModel)
{
	x = precisionModel->makePrecise(x);
	y = precisionModel->makePrecise(y);
}

inline double
Coordinate::distance(const Coordinate& p) const
{
	double dx = x - p.x;
	double dy = y - p.y;
	return sqrt(dx * dx + dy * dy);
}

inline int
Coordinate::hashCode() const
{
	//Algorithm from Effective Java by Joshua Bloch [Jon Aquino]
	int result = 17;
	result = 37 * result + hashCode(x);
	result = 37 * result + hashCode(y);
	return result;
}

/**
 * Returns a hash code for a double value, using the algorithm from
 * Joshua Bloch's book <i>Effective Java</i>
 */
inline int
Coordinate::hashCode(double x)
{
	int64 f = (int64)(x);
	return (int)(f^(f>>32));
}



//* class CoordinateList geom.h geos.h
//*
//* brief A list of Coordinates, which may be set to prevent
//* repeated coordinates from occuring in the list.
//
//class CoordinateList {
//public:
//	~CoordinateList(){};
//
//	// copy constructor
//	CoordinateList(const CoordinateList &cl);
//
//	// constructor an empty CoordinateList
//	CoordinateList();
//
//	/*
//	 * Constructs a new list from a vector of Coordinates.
//	 * Caller can specify if repeated points are to be removed.
//	 * Default is allowing repeated points.
//	 * Will take ownership of coords.
//	 */
//	CoordinateList(vector<Coordinate> *coords, bool allowRepeted=false);
//
//	// Get a reference to the nth Coordinate 
//	const Coordinate& getCoordinate(int n) const;
//
//	/*
//	 * \brief Add an array of coordinates 
//	 * @param vc The coordinates
//	 * @param allowRepeated if set to false, repeated coordinates
//	 * 	are collapsed
//	 * @return true (as by general collection contract)
//	 */
//	void add(vector<Coordinate>* vc, bool allowRepeated);
//
//	/*
//	 * \brief Add an array of coordinates 
//	 * @param cl The coordinates
//	 * @param allowRepeated if set to false, repeated coordinates
//	 * are collapsed
//	 * @param direction if false, the array is added in reverse order
//	 * @return true (as by general collection contract)
//	 */
//	void add(CoordinateList *cl,bool allowRepeated,bool direction);
//
//	/*
//	 * \brief Add a coordinate
//	 * @param c The coordinate to add
//	 * @param allowRepeated if set to false, repeated coordinates
//	 * are collapsed
//	 * @return true (as by general collection contract)
//	 */
//	void add(const Coordinate& c,bool allowRepeated);
//
//	// Add a Coordinate to the list
//	void add(const Coordinate& c);
//
//	// Get vector
//	const vector<Coordinate>* toCoordinateArray() const;
//
//private:
//
//	vector<Coordinate> *vect;
//};

/**
 * \class CoordinateSequence geom.h geos.h
 *
 * \brief
 * The internal representation of a list of coordinates inside a Geometry.
 *
 * There are some cases in which you might want Geometries to store their
 * points using something other than the GEOS Coordinate class. For example, you
 * may want to experiment with another implementation, such as an array of Xs
 * and an array of Ys. or you might want to use your own coordinate class, one
 * that supports extra attributes like M-values.
 * 
 * You can do this by implementing the CoordinateSequence and
 * CoordinateSequenceFactory interfaces. You would then create a
 * GeometryFactory parameterized by your CoordinateSequenceFactory, and use
 * this GeometryFactory to create new Geometries. All of these new Geometries
 * will use your CoordinateSequence implementation.
 * 
 */
class Envelope;
class CoordinateSequence {
public:
	virtual ~CoordinateSequence(){};

	/** \brief
	 * Returns a deep copy of this collection.
	 */
	virtual CoordinateSequence *clone() const=0;

	/** \brief
	 * Returns a read-only reference to Coordinate at position i.
	 *
	 * Whether or not the Coordinate returned is the actual underlying
	 * Coordinate or merely a copy depends on the implementation.
	 */
	//virtual const Coordinate& getCoordinate(int i) const=0;
	virtual const Coordinate& getAt(int i) const=0;

	/** \brief
	 * Returns the number of Coordinates (actual or otherwise, as
	 * this implementation may not store its data in Coordinate objects).
	 */
	//virtual int size() const=0;
	virtual int getSize() const=0;

	/** \brief
	 * Returns a read-only vector with the Coordinates in this collection.
	 *
	 * Whether or not the Coordinates returned are the actual underlying
	 * Coordinates or merely copies depends on the implementation.
	 * Note that if this implementation does not store its data as an
	 * array of Coordinates, this method will incur a performance penalty
	 * because the array needs to be built from scratch.
	 */
	virtual	const vector<Coordinate>* toVector() const=0;

	/**
	 * \brief Add an array of coordinates 
	 * @param vc The coordinates
	 * @param allowRepeated if set to false, repeated coordinates
	 * 	are collapsed
	 * @return true (as by general collection contract)
	 */
	void add(const vector<Coordinate>* vc, bool allowRepeated);

	/* This is here for backward compatibility.. */
	void add(CoordinateSequence *cl,bool allowRepeated,bool direction);

	/**
	 * \brief Add an array of coordinates 
	 * @param cl The coordinates
	 * @param allowRepeated if set to false, repeated coordinates
	 * are collapsed
	 * @param direction if false, the array is added in reverse order
	 * @return true (as by general collection contract)
	 */
	void add(const CoordinateSequence *cl,bool allowRepeated,bool direction);

	/**
	 * \brief Add a coordinate
	 * @param c The coordinate to add
	 * @param allowRepeated if set to false, repeated coordinates
	 * are collapsed
	 * @return true (as by general collection contract)
	 */
	void add(const Coordinate& c,bool allowRepeated);

	/// Returns <code>true</code> it list contains no coordinates.
	virtual	bool isEmpty() const=0;

	/// Add a Coordinate to the list
	virtual	void add(const Coordinate& c)=0;

	// Get number of coordinates
	//virtual int getSize() const=0;

	/// Get a reference to Coordinate at position pos
	//virtual	const Coordinate& getAt(int pos) const=0;

	/// Copy Coordinate c to position pos
	virtual	void setAt(const Coordinate& c, int pos)=0;

	/// Delete Coordinate at position pos (list will shrink).
	virtual	void deleteAt(int pos)=0;

	/// Get a string rapresentation of CoordinateSequence
	virtual	string toString() const=0;

	/// Substitute Coordinate list with a copy of the given vector
	virtual	void setPoints(const vector<Coordinate> &v)=0;
	
	/// Returns true if contains any two consecutive points 
	bool hasRepeatedPoints() const;

	/// Returns lower-left Coordinate in list
	const Coordinate* minCoordinate() const;


	/// Returns a new CoordinateSequence being a copy of the input with any consecutive equal Coordinate removed.
	static CoordinateSequence* removeRepeatedPoints(const CoordinateSequence *cl);

	/**
	* \brief Returns true if given CoordinateSequence contains
	* any two consecutive Coordinate 
	*/
	static bool hasRepeatedPoints(const CoordinateSequence *cl);

	/**
	* \brief Returns either the given CoordinateSequence if its length
	* is greater than the given amount, or an empty CoordinateSequence.
	*/
	static CoordinateSequence* atLeastNCoordinatesOrNothing(int n, CoordinateSequence *c);

	/**
	 * \brief Returns lower-left Coordinate in given CoordinateSequence.
	 * This is actually the Coordinate with lower X (and Y if needed)
	 * ordinate.
	 */
	static const Coordinate* minCoordinate(CoordinateSequence *cl);

	/// Return position of a Coordinate, or -1 if not found
	static int indexOf(const Coordinate *coordinate, const CoordinateSequence *cl);
	/**
	* \brief
	* Returns true if the two arrays are identical, both null,
	* or pointwise equal 
	*/
	static bool equals(CoordinateSequence *cl1, CoordinateSequence *cl2);

	/// Scroll given CoordinateSequence so to start with given Coordinate.
	static void scroll(CoordinateSequence *cl, const Coordinate *firstCoordinate);

	/// Reverse Coordinate order in given CoordinateSequence
	static void reverse(CoordinateSequence *cl);

	/// Standard ordinate index values
	enum { X,Y,Z,M };

	/**
	 * Returns the dimension (number of ordinates in each coordinate)
	 * for this sequence.
	 *
	 * @return the dimension of the sequence.
	 */
	virtual int getDimension() const=0;

	/**
	 * Returns the ordinate of a coordinate in this sequence.
	 * Ordinate indices 0 and 1 are assumed to be X and Y.
	 * Ordinates indices greater than 1 have user-defined semantics
	 * (for instance, they may contain other dimensions or measure values).
	 *
	 * @param index  the coordinate index in the sequence
	 * @param ordinateIndex the ordinate index in the coordinate
	 * 	   (in range [0, dimension-1])
	 */
	virtual double getOrdinate(int index, int ordinateIndex) const=0;

	/**
	 * Returns ordinate X (0) of the specified coordinate.
	 *
	 * @param index
	 * @return the value of the X ordinate in the index'th coordinate
	 */
	virtual double getX(int index) const { return getOrdinate(index, X); }

	/**
	 * Returns ordinate Y (1) of the specified coordinate.
	 *
	 * @param index
	 * @return the value of the Y ordinate in the index'th coordinate
	 */
	virtual double getY(int index) const { return getOrdinate(index, Y); }


	/**
	 * Sets the value for a given ordinate of a coordinate in this sequence.
	 *
	 * @param index  the coordinate index in the sequence
	 * @param ordinateIndex the ordinate index in the coordinate
	 * 		(in range [0, dimension-1])
	 * @param value  the new ordinate value
	 */
	virtual void setOrdinate(int index, int ordinateIndex, double value)=0;

	/**
	 * Expands the given Envelope to include the coordinates in the
	 * sequence.
	 * Allows implementing classes to optimize access to coordinate values.
	 *
	 * @param env the envelope to expand
	 */
	virtual void expandEnvelope(Envelope &env) const;


};

/**
 * \class DefaultCoordinateSequence geom.h geos.h
 *
 * \brief The default implementation of CoordinateSequence
 */
class DefaultCoordinateSequence : public CoordinateSequence {
public:

	DefaultCoordinateSequence(const DefaultCoordinateSequence &cl);

	CoordinateSequence *clone() const;

	//const Coordinate& getCoordinate(int pos) const;
	const Coordinate& getAt(int pos) const;

	//int size() const;
	int getSize() const;
	const vector<Coordinate>* toVector() const;

	/// Construct an empty sequence
	DefaultCoordinateSequence();

	/// Construct sequence taking ownership of given Coordinate vector
	DefaultCoordinateSequence(vector<Coordinate> *coords);

	/// Construct sequence allocating space for n coordinates
	DefaultCoordinateSequence(int n);

	~DefaultCoordinateSequence();

	bool isEmpty() const;
	void add(const Coordinate& c);
	void setAt(const Coordinate& c, int pos);
	void deleteAt(int pos);
	string toString() const;
	void setPoints(const vector<Coordinate> &v);

	double getOrdinate(int index, int ordinateIndex) const;
	void setOrdinate(int index, int ordinateIndex, double value);
	void expandEnvelope(Envelope &env) const;
	int getDimension() const { return 3; }

private:
	vector<Coordinate> *vect;
};

struct point_3d {
	double x;
	double y;
	double z;
};

class PointCoordinateSequence : public CoordinateSequence {
public:
	PointCoordinateSequence();
	PointCoordinateSequence(int n);
	PointCoordinateSequence(const Coordinate& c);
	PointCoordinateSequence(const PointCoordinateSequence &cl);
	PointCoordinateSequence(const CoordinateSequence *c);
	virtual ~PointCoordinateSequence();
	CoordinateSequence *clone() const;
	bool isEmpty() const;
	void add(const Coordinate& c);
	void add(point_3d p);
	int getSize() const;
	const Coordinate& getAt(int pos) const;
	point_3d getPointAt(int pos);
	void setAt(const Coordinate& c, int pos);
	void setAt(point_3d p, int pos);
	void deleteAt(int pos);
	const vector<Coordinate>* toVector() const;
	vector<point_3d>* toPointVector();
	string toString() const;
	void setPoints(const vector<Coordinate> &v);
	void setPoints(vector<point_3d> &v);
private:
	vector<point_3d> *vect;
	mutable vector<Coordinate>*cached_vector;
};

/**
 * \class CoordinateSequenceFactory geom.h geos.h
 *
 * \brief
 * An object that knows how to build a particular implementation of
 * CoordinateSequence from an array of Coordinates.
 */
class CoordinateSequenceFactory {
public:

	/** \brief
	 * Returns a CoordinateSequence based on the given array.
	 * Whether or not the vector is copied is implementation-dependent,
	 * for this reason caller does give up ownership of it.
	 * Implementations that will not copy it will need take care
	 * of deleting it.
	 * Note that a NULL value is allowed as coordinates, and will
	 * create an empty CoordinateSequence.
	 */
	virtual CoordinateSequence *create(vector<Coordinate> *coordinates) const=0;

	/** \brief
	 * Creates a CoordinateSequence of the specified size and dimension.
	 * For this to be useful, the CoordinateSequence implementation must
	 * be mutable.
	 *
	 * @param size the number of coordinates in the sequence
	 * @param dimension the dimension of the coordinates in the sequence
	 * 	(if user-specifiable, otherwise ignored)
	 */
	virtual CoordinateSequence *create(unsigned int size, int dimension)
		const=0;
};

/**
 * \class DefaultCoordinateSequenceFactory geom.h geos.h
 *
 * \brief
 * Creates CoordinateSequences internally represented as an array of
 * Coordinates.
 */
class DefaultCoordinateSequenceFactory: public CoordinateSequenceFactory {

public:

	/** \brief
	 * Returns a DefaultCoordinateSequence based on the given vector
	 * (the vector is not copied - callers give up ownership).
	 */
	CoordinateSequence *create(vector<Coordinate> *coords) const
	{
		return new DefaultCoordinateSequence(coords);
	}

   	/** @see CoordinateSequenceFactory::create(unsigned int, int) */
	CoordinateSequence *create(unsigned int size, int dimension=3) const
	{
		/* DefaultCoordinateSequence only accepts 3d Coordinates */
		return new DefaultCoordinateSequence(size);
	}

	/** \brief
	 * Returns the singleton instance of DefaultCoordinateSequenceFactory
	 */
	static const CoordinateSequenceFactory *instance();
};

/*
 * \class PointCoordinateSequenceFactory geom.h geos.h
 *
 * \brief
 * Factory for PointCoordinateSequence objects.
 */
class PointCoordinateSequenceFactory: public CoordinateSequenceFactory {
public:

	CoordinateSequence *create(vector<Coordinate> *coords) const;
};

/*
 * <code>Geometry</code> classes support the concept of applying a
 * coordinate filter to every coordinate in the <code>Geometry</code>. A
 * coordinate filter can either record information about each coordinate or
 * change the coordinate in some way. Coordinate filters implement the
 * interface <code>CoordinateFilter</code>. (<code>CoordinateFilter</code> is
 * an example of the Gang-of-Four Visitor pattern). Coordinate filters can be
 * used to implement such things as coordinate transformations, centroid and
 * envelope computation, and many other functions.
 *
 */
class CoordinateFilter {
public:
   virtual ~CoordinateFilter() {}
   /**
    *  Performs an operation with or on <code>coord</code>.
    *
    *@param  coord  a <code>Coordinate</code> to which the filter is applied.
    */
   virtual void filter_rw(Coordinate* coord)=0;
   virtual void filter_ro(const Coordinate* coord)=0;
};

class Geometry;

/*
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


/*
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
 * \class Envelope geom.h geos.h
 *
 * \brief
 * An Envelope defines a rectangulare region of the 2D coordinate plane.
 *
 * It is often used to represent the bounding box of a Geometry,
 * e.g. the minimum and maximum x and y values of the Coordinates.
 *  
 * Note that Envelopes support infinite or half-infinite regions, by using
 * the values of <code>Double_POSITIVE_INFINITY</code> and
 * <code>Double_NEGATIVE_INFINITY</code>.
 *
 * When Envelope objects are created or initialized,
 * the supplies extent values are automatically sorted into the correct order.
 *
 */
class Envelope {
public:
	Envelope(void);
	Envelope(double x1, double x2, double y1, double y2);
	Envelope(const Coordinate& p1, const Coordinate& p2);
	Envelope(const Coordinate& p);
	Envelope(const Envelope &env);
	~Envelope(void);
	static bool intersects(const Coordinate& p1,const Coordinate& p2,const Coordinate& q);
	static bool intersects(const Coordinate& p1,const Coordinate& p2,const Coordinate& q1,const Coordinate& q2);
	void init(void);
	void init(double x1, double x2, double y1, double y2);
	void init(const Coordinate& p1, const Coordinate& p2);
	void init(const Coordinate& p);
	void init(Envelope env);
	void setToNull(void);

	inline bool isNull(void) const;

	double getWidth(void) const;
	double getHeight(void) const;
	inline double getMaxY() const;
	inline double getMaxX() const;
	inline double getMinY() const;
	inline double getMinX() const;
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

	inline bool intersects(const Envelope* other) const;

	bool equals(const Envelope* other) const;
	string toString(void) const;
	double distance(const Envelope* env) const;
	int hashCode() const;

private:
	static double distance(double x0,double y0,double x1,double y1);
	double minx;	/// the minimum x-coordinate
	double maxx;	/// the maximum x-coordinate
	double miny;	/// the minimum y-coordinate
	double maxy;	/// the maximum y-coordinate
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID=5873921885273102420I64;
#else        
	static const int64 serialVersionUID=5873921885273102420LL;
#endif        
};

inline double Envelope::getMaxY() const { return maxy; }
inline double Envelope::getMaxX() const { return maxx; }
inline double Envelope::getMinY() const { return miny; }
inline double Envelope::getMinX() const { return minx; }

inline bool
Envelope::isNull(void) const
{
	return maxx < minx;
}

inline bool
Envelope::intersects(const Envelope* other) const
{
	// Optimized to reduce function calls
	if ( isNull() || other->isNull() ) return false;
	return !(other->minx > maxx ||
			 other->maxx < minx ||
			 other->miny > maxy ||
			 other->maxy < miny);
}

class Geometry;
class GeometryFilter;
class IntersectionMatrix;


class CGAlgorithms;
class Point;
class GeometryFactory;

/**
 * \class Geometry geom.h geos.h
 *
 * \brief Basic implementation of Geometry, constructed and
 * destructed by GeometryFactory.
 *
 *  <code>clone</code> returns a deep copy of the object.
 *  Use GeometryFactory to construct.
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

	Geometry(const Geometry &geom);

	/** \brief
	 * Construct a geometry with the given GeometryFactory.
	 * Will keep a reference to the factory, so don't
	 * delete it until al Geometry objects referring to
	 * it are deleted.
	 */
	Geometry(const GeometryFactory *factory);

	/** Destroy Geometry and all components */
	virtual ~Geometry();

	/// Make a deep-copy of this Geometry
	virtual Geometry* clone() const=0;

	/**
	 * \brief
	 * Gets the factory which contains the context in which this
	 * geometry was created.
	 *
	 * @return the factory for this geometry
	 */
	const GeometryFactory* getFactory() const;

	/**
	* \brief
	* A simple scheme for applications to add their own custom data to
	* a Geometry.
	* An example use might be to add an object representing a
	* Coordinate Reference System.
	* 
	* Note that user data objects are not present in geometries created
	* by construction methods.
	*
	* @param newUserData an object, the semantics for which are
	* defined by the application using this Geometry
	*/
	void setUserData(void* newUserData);

	/**
	* \brief
	* Gets the user data object for this geometry, if any.
	*
	* @return the user data object, or <code>null</code> if none set
	*/
	void* getUserData();

	/*
	 * \brief
	 * Returns the ID of the Spatial Reference System used by the
	 * <code>Geometry</code>.
	 *
	 * GEOS supports Spatial Reference System information in the simple way
	 * defined in the SFS. A Spatial Reference System ID (SRID) is present
	 * in each <code>Geometry</code> object. <code>Geometry</code>
	 * provides basic accessor operations for this field, but no others.
	 * The SRID is represented as an integer.
	 *
	 * @return the ID of the coordinate space in which the
	 * <code>Geometry</code> is defined.
	 *
	 * @deprecated use getUserData instead
	 */
	virtual int getSRID() const;

	/*
	 * Sets the ID of the Spatial Reference System used by the
	 * <code>Geometry</code>.
	 * @deprecated use setUserData instead
	 */
	virtual void setSRID(int newSRID);

	/**
	 * \brief
	 * Get the PrecisionModel used to create this Geometry.
	 */
	virtual const PrecisionModel* getPrecisionModel() const;

	/// Returns a vertex of this Geometry.
	virtual const Coordinate* getCoordinate() const=0; //Abstract

	/**
	 * \brief
	 * Returns this Geometry vertices.
	 * Caller takes ownership of the returned object.
	 */
	virtual CoordinateSequence* getCoordinates() const=0; //Abstract

	/// Returns the count of this Geometrys vertices.
	virtual int getNumPoints() const=0; //Abstract

	/// Returns false if the Geometry not simple.
	virtual bool isSimple() const=0; //Abstract

	/// Return a string representation of this Geometry type
	virtual string getGeometryType() const=0; //Abstract

	/// Return an integer representation of this Geometry type
	virtual GeometryTypeId getGeometryTypeId() const=0; //Abstract

	/**
	 * \brief Tests the validity of this <code>Geometry</code>.
	 *
	 * Subclasses provide their own definition of "valid".
	 *
	 * @return <code>true</code> if this <code>Geometry</code> is valid
	 *
	 * @see IsValidOp
	 */
	virtual bool isValid() const;

	/// Returns whether or not the set of points in this Geometry is empty.
	virtual bool isEmpty() const=0; //Abstract

	/// Returns the dimension of this Geometry (0=point, 1=line, 2=surface)
	virtual int getDimension() const=0; //Abstract

	/**
	 * \brief
	 * Returns the boundary, or the empty geometry if this Geometry
	 * is empty.
	 */
	virtual Geometry* getBoundary() const=0; //Abstract

	/// Returns the dimension of this Geometrys inherent boundary.
	virtual int getBoundaryDimension() const=0; //Abstract

	/// Returns this Geometrys bounding box.
	virtual Geometry* getEnvelope() const;

	/** \brief
	 * Returns the minimum and maximum x and y values in this Geometry,
	 * or a null Envelope if this Geometry is empty.
	 */
	virtual const Envelope* getEnvelopeInternal() const;

	/**
	 * \brief
	 * Returns true if the DE-9IM intersection matrix for the
	 * two Geometrys is FF*FF****.
	 */
	virtual bool disjoint(const Geometry *other) const;

	/** \brief
	 * Returns true if the DE-9IM intersection matrix for the two
	 * Geometrys is FT*******, F**T***** or F***T****.
	 */
	virtual bool touches(const Geometry *other) const;

	/// Returns true if disjoint returns false.
	virtual bool intersects(const Geometry *g) const;

	/**
	 * \brief
	 * Returns true if the DE-9IM intersection matrix for the two
	 * Geometrys is T*T****** (for a point and a curve, a point and
	 * an area or a line and an area) 0******** (for two curves).
	 */
	virtual bool crosses(const Geometry *g) const;

	/** \brief
	 * Returns true if the DE-9IM intersection matrix for the two
	 * Geometrys is T*F**F***.
	 */
	virtual bool within(const Geometry *g) const;

	/// Returns true if other.within(this) returns true.
	virtual bool contains(const Geometry *g) const;

	/** \brief
	 * Returns true if the DE-9IM intersection matrix for the two
	 * Geometrys is T*T***T** (for two points or two surfaces)
	 * 1*T***T** (for two curves).
	 */
	virtual bool overlaps(const Geometry *g) const;

	/**
	 * \brief
	 * Returns true if the elements in the DE-9IM intersection matrix
	 * for the two Geometrys match the elements in intersectionPattern.
	 *
	 * IntersectionPattern elements may be: 0 1 2 T ( = 0, 1 or 2)
	 * F ( = -1) * ( = -1, 0, 1 or 2).
	 *
	 * For more information on the DE-9IM, see the OpenGIS Simple
	 * Features Specification.
	 */
	virtual bool relate(const Geometry *g, string intersectionPattern) const;
	/// Returns the DE-9IM intersection matrix for the two Geometrys.
	virtual IntersectionMatrix* relate(const Geometry *g) const;

	/**
	 * \brief
	 * Returns true if the DE-9IM intersection matrix for the two
	 * Geometrys is T*F**FFF*.
	 */
	virtual bool equals(const Geometry *g) const;

	/// Returns the Well-known Text representation of this Geometry.
	virtual string toString() const;

	virtual string toText() const;
	
	/// Returns a buffer region around this Geometry having the given width.
	virtual Geometry* buffer(double distance) const;

	/// Returns a buffer region around this Geometry having the given width and with a specified number of segments used to approximate curves.
	virtual Geometry* buffer(double distance,int quadrantSegments) const;

	/// Returns the smallest convex Polygon that contains all the points in the Geometry.
	virtual Geometry* convexHull() const;

	/** \brief
	 * Returns a Geometry representing the points shared by
	 * this Geometry and other.
	 */
	virtual Geometry* intersection(const Geometry *other) const;

	/** \brief
	 * Returns a Geometry representing all the points in this Geometry
	 * and other.
	 */
	virtual Geometry* Union(const Geometry *other) const;
		// throw(IllegalArgumentException *, TopologyException *);

	/**
	 * \brief
	 * Returns a Geometry representing the points making up this
	 * Geometry that do not make up other.
	 */
	virtual Geometry* difference(const Geometry *other) const;

	/** \brief
	 * Returns a set combining the points in this Geometry not in other,
	 * and the points in other not in this Geometry.
	 */
	virtual Geometry* symDifference(const Geometry *other) const;

	/** \brief
	 * Returns true if the two Geometrys are exactly equal,
	 * up to a specified tolerance.
	 */
	virtual bool equalsExact(const Geometry *other, double tolerance)
		const=0; //Abstract

	virtual void apply_rw(CoordinateFilter *filter)=0; //Abstract
	virtual void apply_ro(CoordinateFilter *filter) const=0; //Abstract
	virtual void apply_rw(GeometryFilter *filter);
	virtual void apply_ro(GeometryFilter *filter) const;
	virtual void apply_rw(GeometryComponentFilter *filter);
	virtual void apply_ro(GeometryComponentFilter *filter) const;

	/// Converts this Geometry to normal form (or  canonical form).
	virtual void normalize()=0; //Abstract

	virtual int compareTo(const Geometry *geom) const;

	/** \brief
	 * Returns the minimum distance between this Geometry
	 * and the Geometry g
	 */
	virtual double distance(const Geometry *g) const;

	/// Returns the area of this Geometry.
	virtual double getArea() const;

	/// Returns the length of this Geometry.
	virtual double getLength() const;

	/** \brief
	 * Tests whether the distance from this Geometry  to another
	 * is less than or equal to a specified value.
	 */
	virtual bool isWithinDistance(const Geometry *geom,double cDistance);

	/// Computes the centroid of this Geometry.
	virtual Point* getCentroid() const;

	/// Computes an interior point of this Geometry.
	virtual Point* getInteriorPoint();

	/*
	 * \brief
	 * Notifies this Geometry that its Coordinates have been changed
	 * by an external party (using a CoordinateFilter, for example).
	 */
	virtual void geometryChanged();

	/*
	 * \brief
	 * Notifies this Geometry that its Coordinates have been changed
	 * by an external party.
	 */
	void geometryChangedAction();

protected:
	mutable Envelope* envelope;
	
	/// Returns true if the array contains any non-empty Geometrys.
	static bool hasNonEmptyElements(const vector<Geometry *>* geometries);

	/// Returns true if the CoordinateSequence contains any null elements.
	static bool hasNullElements(const CoordinateSequence* list);

	/// Returns true if the vector contains any null elements.
	static bool hasNullElements(const vector<Geometry *>* lrs);

//	static void reversePointOrder(CoordinateSequence* coordinates);
//	static Coordinate& minCoordinate(CoordinateSequence* coordinates);
//	static void scroll(CoordinateSequence* coordinates,Coordinate* firstCoordinate);
//	static int indexOf(Coordinate* coordinate,CoordinateSequence* coordinates);
//
	/** \brief
	 * Returns whether the two Geometrys are equal, from the point
	 * of view of the equalsExact method.
	 */
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
	* The GEOS algorithms assume that Geometry::getCoordinate() and
	* #getCoordinates
	* are fast, which may not be the case if the CoordinateSequence is not a
	* DefaultCoordinateSequence (e.g. if it were implemented using separate
	* arrays for the x- and y-values), in which case frequent
	* construction of Coordinates takes up much space and time.
	* To solve this performance problem, toInternalGeometry converts the
	* Geometry to a DefaultCoordinateSequence
	* implementation before sending it
	* to the JTS algorithms.
	*/
	Geometry* toInternalGeometry(const Geometry *g) const;
	Geometry* fromInternalGeometry(const Geometry *g) const;
private:
	virtual int getClassSortIndex() const;
	static GeometryComponentFilter geometryChangedFilter;
#ifdef INT64_CONST_IS_I64
    static const int64 serialVersionUID = 8763622679187376702I64;
#else
    static const int64 serialVersionUID = 8763622679187376702LL;
#endif
	const GeometryFactory *factory;
	static const GeometryFactory* INTERNAL_GEOMETRY_FACTORY;
	void* userData;
	Point* createPointFromInternalCoord(const Coordinate* coord,const Geometry *exemplar) const;
};

/*
 * Geometry classes support the concept of applying a Geometry
 * filter to the Geometry. In the case of GeometryCollection
 * subclasses, the filter is applied to every element Geometry.
 * A Geometry filter can either record information about the Geometry
 * or change the Geometry in some way.
 * Geometry filters implement the interface GeometryFilter.
 * (GeometryFilter is an example of the Gang-of-Four Visitor pattern).
 */
class GeometryFilter {
public:
	/*
	 * Performs an operation with or on <code>geom</code>.
	 *
	 * @param  geom  a <code>Geometry</code> to which the filter
	 *         is applied.
	 */
	virtual void filter_ro(const Geometry *geom)=0;
	virtual void filter_rw(Geometry *geom)=0;
};

/*
 * Represents a line segment defined by two {@link Coordinate}s.
 * Provides methods to compute various geometric properties
 * and relationships of line segments.
 * 
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
	* The returned CoordinateList must be deleted by caller
	*/
	virtual CoordinateSequence* closestPoints(const LineSegment *line);

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
#ifdef INT64_CONST_IS_I64
  static const int64 serialVersionUID=3252005833466256227I64;
#else
  static const int64 serialVersionUID=3252005833466256227LL;
#endif

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

/*
 *  Constants representing the location of a point relative to a geometry. They
 *  can also be thought of as the row or column index of a DE-9IM matrix. For a
 *  description of the DE-9IM, see the <A
 *  HREF="http://www.opengis.org/techno/specs.htm">OpenGIS Simple Features
 *  Specification for SQL</A> .
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
bool operator!=(const Coordinate& a, const Coordinate& b);
bool operator==(const Envelope a, const Envelope b);
bool operator==(const PrecisionModel a, const PrecisionModel b);
bool operator==(const LineSegment a, const LineSegment b);

bool lessThen(Coordinate& a,Coordinate& b);
bool greaterThen(Geometry *first, Geometry *second);

/**
 * \class GeometryCollection geom.h geos.h
 *
 * \brief Represents a collection of heterogeneous Geometry objects.
 *
 * Collections of Geometry of the same type are 
 * represented by GeometryCollection subclasses MultiPoint,
 * MultiLineString, MultiPolygon.
 */
class GeometryCollection : public Geometry{
public:
	GeometryCollection(const GeometryCollection &gc);

	/** \brief
	 * Construct a GeometryCollection with the given GeometryFactory.
	 * Will keep a reference to the factory, so don't
	 * delete it until al Geometry objects referring to
	 * it are deleted.
	 * Will take ownership of the Geometry vector.
	 *
	 * @param newGeoms
	 *	The <code>Geometry</code>s for this
	 *	<code>GeometryCollection</code>,
	 *	or <code>null</code> or an empty array to
	 *	create the empty geometry.
	 *	Elements may be empty <code>Geometry</code>s,
	 *	but not <code>null</code>s.
	 *
	 *	If construction succeed the created object will take
	 *	ownership of newGeoms vector and elements.
	 *
	 *	If construction	fails "IllegalArgumentException *"
	 *	is thrown and it is your responsibility to delete newGeoms
	 *	vector and content.
	 *
	 * @param newFactory the GeometryFactory used to create this geometry
	 */
	GeometryCollection(vector<Geometry *> *newGeoms, const GeometryFactory *newFactory);

	virtual Geometry *clone() const;

	virtual ~GeometryCollection();

	/**
	 * \brief
	 * Collects all coordinates of all subgeometries into a
	 * CoordinateSequence.
	 * 
	 * Note that the returned coordinates are copies, so
	 * you want be able to use them to modify the geometries
	 * in place. Also you'll need to delete the CoordinateSequence
	 * when finished using it.
	 * 
	 * @return the collected coordinates
	 *
	 */
	virtual CoordinateSequence* getCoordinates() const;

	virtual bool isEmpty() const;

	/**
	 * \brief
	 * Returns the maximum dimension of geometries in this collection
	 * (0=point, 1=line, 2=surface)
	 */
	virtual int getDimension() const;

	virtual Geometry* getBoundary() const;

	/**
	 * \brief
	 * Returns the maximum boundary dimension of geometries in
	 * this collection.
	 */
	virtual int getBoundaryDimension() const;

	virtual int getNumPoints() const;
	virtual string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	virtual bool isSimple() const;
	virtual bool equalsExact(const Geometry *other, double tolerance) const;

	virtual void apply_ro(CoordinateFilter *filter) const;
	virtual void apply_rw(CoordinateFilter *filter);
	virtual void apply_ro(GeometryFilter *filter) const;
	virtual void apply_rw(GeometryFilter *filter);
	virtual void apply_ro(GeometryComponentFilter *filter) const;
	virtual void apply_rw(GeometryComponentFilter *filter);
	virtual void normalize();
	virtual const Coordinate* getCoordinate() const;
	/// Returns the total area of this collection
	virtual double getArea() const;
	/// Returns the total length of this collection
	virtual double getLength() const;
	/// Returns the number of geometries in this collection
	virtual int getNumGeometries() const;
	/// Returns a pointer to the nth Geometry int this collection
	virtual const Geometry* getGeometryN(int n) const;
protected:
	vector<Geometry *>* geometries;
	virtual Envelope* computeEnvelopeInternal() const;
	virtual int compareToSameClass(const Geometry *gc) const;
private:
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = -5694727726395021467I64;
#else
	static const int64 serialVersionUID = -5694727726395021467LL;
#endif        
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
 * \class Point geom.h geos.h
 * \brief Basic implementation of Point.
 */
class Point : public Geometry{
public:

	/**
	 * \brief
	 * Creates a Point taking ownership of the given CoordinateSequence
	 * (must have 1 element)
	 *
	 * @param  newCoords
	 *	contains the single coordinate on which to base this
	 *	<code>Point</code> or <code>null</code> to create
	 *	the empty geometry.
	 *
	 * @param newFactory the GeometryFactory used to create this geometry
	 */  
	Point(CoordinateSequence *newCoords, const GeometryFactory *newFactory);

	Point(const Point &p); 
	virtual ~Point();
	Geometry *clone() const;
	CoordinateSequence* getCoordinates(void) const;
	const CoordinateSequence* getCoordinatesRO() const {
		return coordinates;
	}
	int getNumPoints() const;
	bool isEmpty() const;
	bool isSimple() const;
	//bool isValid() const;

	/// Returns point dimension (0)
	int getDimension() const;

	/// Returns Dimension::False (Point has no boundary)
	int getBoundaryDimension() const;

	/// Returns an EMPTY Geometry.
	Geometry* getBoundary() const;

	double getX() const;
	double getY() const;
	const Coordinate* getCoordinate() const;
	string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
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
	CoordinateSequence *coordinates;
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = 4902022702746614570I64;
#else        
	static const int64 serialVersionUID = 4902022702746614570LL;
#endif        
};

/**
 * \class LineString geom.h geos.h
 * \brief Basic implementation of LineString.
 */
class LineString: public Geometry {
public:
	LineString(const LineString &ls);

	/// Constructs a LineString taking ownership the given CoordinateSequence.
	LineString(CoordinateSequence *pts, const GeometryFactory *newFactory);

	virtual ~LineString();
	virtual Geometry *clone() const;
	virtual CoordinateSequence* getCoordinates() const;

	/// Returns a read-only pointer to internal CoordinateSequence
	const CoordinateSequence* getCoordinatesRO() const;

	virtual const Coordinate& getCoordinateN(int n) const;

	/// Returns line dimension (1)
	virtual int getDimension() const;

	/**
	 * \brief
	 * Returns Dimension::False for a closed LineString,
	 * 0 otherwise (LineString boundary is a MultiPoint)
	 */
	virtual int getBoundaryDimension() const;

	/**
	 * \brief
	 * Returns a MultiPoint.
	 * Empty for closed LineString, a Point for each vertex otherwise.
	 */
	virtual Geometry* getBoundary() const;

	virtual bool isEmpty() const;
	virtual int getNumPoints() const;
	virtual Point* getPointN(int n) const;
	virtual Point* getStartPoint() const;
	virtual Point* getEndPoint() const;
	virtual bool isClosed() const;
	virtual bool isRing() const;
	virtual string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	virtual bool isSimple() const;
	virtual bool isCoordinate(Coordinate& pt) const;
	virtual bool equalsExact(const Geometry *other, double tolerance) const;
	virtual void apply_rw(CoordinateFilter *filter);
	virtual void apply_ro(CoordinateFilter *filter) const;
	virtual void apply_rw(GeometryFilter *filter);
	virtual void apply_ro(GeometryFilter *filter) const;
	virtual void apply_rw(GeometryComponentFilter *filter);
	virtual void apply_ro(GeometryComponentFilter *filter) const;

	/// Normalize a LineString.  
	virtual void normalize();

	//was protected
	virtual int compareToSameClass(const Geometry *ls) const;
	virtual int compareTo(const LineString *ls) const;
	virtual const Coordinate* getCoordinate() const;
	virtual double getLength() const;
protected:
	virtual Envelope* computeEnvelopeInternal() const;
	CoordinateSequence* points;
private:
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = 3110669828065365560I64;
#else
	static const int64 serialVersionUID = 3110669828065365560LL;
#endif        
};

/**
 * \class LinearRing geom.h geos.h
 *
 * \brief Basic implementation of <code>LinearRing</code>.
 *
 * The first and last point in the coordinate sequence must be equal.
 * Either orientation of the ring is allowed.
 * A valid ring must not self-intersect.
 *
 */
class LinearRing : public LineString{

public:

	LinearRing(const LinearRing &lr);

	/**
	* \brief Constructs a <code>LinearRing</code> with the given points.
	*
	* @param  points  points forming a closed and simple linestring, or
	*      <code>null</code> or an empty array to create the empty geometry.
	*      This array must not contain <code>null</code> elements.
	*	If not null LinearRing will take ownership of points.
	*
	* @param newFactory the GeometryFactory used to create this geometry
	*
	*/
	LinearRing(CoordinateSequence* points, const GeometryFactory *newFactory);

	virtual Geometry *clone() const;
	virtual ~LinearRing();
	bool isSimple() const;
	string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	bool isClosed() const;
	void setPoints(CoordinateSequence* cl);
private:
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = -4261142084085851829I64;
#else
	static const int64 serialVersionUID = -4261142084085851829LL;
#endif        
	void validateConstruction();
};

/**
 * \class Polygon geom.h geos.h
 *
 * \brief Represents a linear polygon, which may include holes.
 *
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
	Polygon(const Polygon &p);
	virtual ~Polygon();

	/**
	 * Constructs a <code>Polygon</code> with the given exterior 
	 * and interior boundaries.
	 *
	 * @param  newShell  the outer boundary of the new Polygon,
	 *                   or <code>null</code> or an empty
	 *		     LinearRing if the empty geometry
	 *                   is to be created.
	 *
	 * @param  newHoles  the LinearRings defining the inner
	 *                   boundaries of the new Polygon, or
	 *                   null or empty LinearRing 
	 *                   if the empty  geometry is to be created.
	 *
	 * @param newFactory the GeometryFactory used to create this geometry
	 *
	 * Polygon will take ownership of Shell and Holes LinearRings 
	 */
	Polygon(LinearRing *newShell, vector<Geometry *> *newHoles,
		const GeometryFactory *newFactory);

	virtual Geometry *clone() const;
	CoordinateSequence* getCoordinates() const;
	int getNumPoints() const;

	/// Returns surface dimension (2)
	int getDimension() const;

	/// Returns 1 (Polygon boundary is a MultiLineString)
	int getBoundaryDimension() const;

	/**
	 * \brief
	 * Returns a MultiLineString.
	 * One LineString for the shell and one for each hole.
	 * Empty for an empty Polygon.
	 */
	Geometry* getBoundary() const;

	bool isEmpty() const;
	bool isSimple() const;
	
	/// Returns the exterior ring (shell)
	const LineString* getExteriorRing() const;

	/// Returns number of interior rings (hole)
	int getNumInteriorRing() const;

	/// Get nth interior ring (hole)
	const LineString* getInteriorRingN(int n) const;

	string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
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

 	/// Returns the perimeter of this <code>Polygon</code>
	double getLength() const;

	void apply_rw(GeometryComponentFilter *filter);
	void apply_ro(GeometryComponentFilter *filter) const;
protected:
	LinearRing *shell;
	vector<Geometry *> *holes; //Actually vector<LinearRing *>
	Envelope* computeEnvelopeInternal() const;
private:
	void normalize(LinearRing *ring, bool clockwise);
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = -3494792200821764533I64;
#else
	static const int64 serialVersionUID = -3494792200821764533LL;
#endif        
};

/**
 * \class MultiPoint geom.h geos.h
 * \brief  Models a collection of Point objects.
 */
class MultiPoint: public GeometryCollection{
public:

	/**
	 * \brief Constructs a <code>MultiPoint</code>.
	 *
	 * @param  newPoints
	 *	the <code>Point</code>s for this <code>MultiPoint</code>,
	 *	or <code>null</code> or an empty array to create the empty
	 * 	geometry.
	 *	Elements may be empty <code>Point</code>s,
	 *	but not <code>null</code>s.
	 *
	 *	Constructed object will take ownership of
	 *	the vector and its elements.
	 *
	 * @param newFactory
	 * 	The GeometryFactory used to create this geometry
	 *	Caller must keep the factory alive for the life-time
	 *	of the constructed MultiPoint.
	 */
	MultiPoint(vector<Geometry *> *newPoints, const GeometryFactory *newFactory);

	virtual ~MultiPoint();

	/// Returns point dimension (0)
	int getDimension() const;

	/// Returns Dimension::False (Point has no boundary)
	int getBoundaryDimension() const;

	/// Returns an EMPTY Geometry
	Geometry* getBoundary() const;

	string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	//bool isValid() const;
	bool isSimple() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
protected:
	const Coordinate* getCoordinate(int n) const;
private:
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = -8048474874175355449I64;
#else
	static const int64 serialVersionUID = -8048474874175355449LL;
#endif        
};

/**
 * \class MultiLineString geom.h geos.h
 * \brief Basic implementation of MultiLineString objects.
 */
class MultiLineString: public GeometryCollection{
public:

	/**
	 * \brief Constructs a <code>MultiLineString</code>.
	 *
	 * @param  newLines
	 *	The <code>LineStrings</code>s for this
	 *	<code>MultiLineString</code>, or <code>null</code>
	 *	or an empty array to create the empty geometry.
	 *	Elements may be empty <code>LineString</code>s,
	 *	but not <code>null</code>s.
	 *
	 *	Constructed object will take ownership of
	 *	the vector and its elements.
	 *
	 * @param newFactory
	 * 	The GeometryFactory used to create this geometry.
	 *	Caller must keep the factory alive for the life-time
	 *	of the constructed MultiLineString.
	 * 	
	 */
	MultiLineString(vector<Geometry *> *newLines, const GeometryFactory *newFactory);

	virtual ~MultiLineString();

	/// Returns line dimension (1)
	int getDimension() const;

	/**
	 * \brief
	 * Returns Dimension::False if all LineStrings in the collection
	 * are closed, 0 otherwise.
	 */
	int getBoundaryDimension() const;

	/// Returns a (possibly empty) MultiPoint 
	Geometry* getBoundary() const;

	string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	bool isClosed() const;
	bool isSimple() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
private:
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = 8166665132445433741I64;
#else
	static const int64 serialVersionUID = 8166665132445433741LL;
#endif        
};

/**
 * \class MultiPolygon  geom.h geos.h
 * \brief Basic implementation of <code>MultiPolygon</code>.
 */
class MultiPolygon: public GeometryCollection {

public:

	/**
	 * \brief Construct a MultiPolygon
	 *
	 * @param newPolys
	 *	the <code>Polygon</code>s for this <code>MultiPolygon</code>,
	 *	or <code>null</code> or an empty array to create the empty
	 *	geometry. Elements may be empty <code>Polygon</code>s, but
	 *	not <code>null</code>s.
	 *	The polygons must conform to the assertions specified in the
	 *	<A HREF="http://www.opengis.org/techno/specs.htm">
	 *	OpenGIS Simple Features Specification for SQL
	 *	</A>.
	 *
	 *	Constructed object will take ownership of
	 *	the vector and its elements.
	 *
	 * @param newFactory
	 * 	The GeometryFactory used to create this geometry
	 *	Caller must keep the factory alive for the life-time
	 *	of the constructed MultiPolygon.
	 */
	MultiPolygon(vector<Geometry *> *newPolys, const GeometryFactory *newFactory);

	virtual ~MultiPolygon();

	/// Returns surface dimension (2)
	int getDimension() const;

	/// Returns 1 (MultiPolygon boundary is MultiLineString)
	int getBoundaryDimension() const;

	/**
	 * \brief
	 * Returns a MultiLineString composed of one LineString for
	 * each of the composing Polygon's shells and holes.
	 */
	Geometry* getBoundary() const;

	string getGeometryType() const;
	virtual GeometryTypeId getGeometryTypeId() const;
	bool isSimple() const;
	bool equalsExact(const Geometry *other, double tolerance) const;
private:
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = -551033529766975875I64;
#else
	static const int64 serialVersionUID = -551033529766975875LL;
#endif        
};

/**
 * \class GeometryFactory geom.h geos.h
 * \brief Supplies a set of utility methods for building Geometry objects
 * from CoordinateSequence or other Geometry objects.
 */
class GeometryFactory {
public:
	/**
	* \brief 
	* Constructs a GeometryFactory that generates Geometries having a
	* floating PrecisionModel and a spatial-reference ID of 0.
	*/
	GeometryFactory();

	/**
	* \brief
	* Constructs a GeometryFactory that generates Geometries having
	* the given PrecisionModel, spatial-reference ID, and
	* CoordinateSequence implementation.
	*/
	GeometryFactory(const PrecisionModel *pm, int newSRID, CoordinateSequenceFactory *nCoordinateSequenceFactory);

	/**
	* \brief
	* Constructs a GeometryFactory that generates Geometries having the
	* given CoordinateSequence implementation, a double-precision floating
	* PrecisionModel and a spatial-reference ID of 0.
	*/
	GeometryFactory(CoordinateSequenceFactory *nCoordinateSequenceFactory);

	/**
	* \brief
	* Constructs a GeometryFactory that generates Geometries having
	* the given PrecisionModel and the default CoordinateSequence
	* implementation.
	*
	* @param pm the PrecisionModel to use
	*/
	GeometryFactory(const PrecisionModel *pm);

	/**
	* \brief
	* Constructs a GeometryFactory that generates Geometries having
	* the given {@link PrecisionModel} and spatial-reference ID,
	* and the default CoordinateSequence implementation.
	*
	* @param pm the PrecisionModel to use
	* @param newSRID the SRID to use
	*/
	GeometryFactory(const PrecisionModel* pm, int newSRID);

	/**
	* \brief Copy constructor
	*
	* @param gf the GeometryFactory to clone from
	*/
	GeometryFactory(const GeometryFactory &gf);

	/// Destructor
	virtual ~GeometryFactory();

//Skipped a lot of list to array convertors

	Point* createPointFromInternalCoord(const Coordinate* coord, const Geometry *exemplar) const;

	/// Envelope to Geometry converter
	Geometry* toGeometry(const Envelope* envelope) const;

	/// Returns the PrecisionModel that Geometries created by this factory will be associated with.
	const PrecisionModel* getPrecisionModel() const;

	/// Creates an EMPTY Point
	Point* createPoint() const;

	/// Creates a Point using the given Coordinate
	Point* createPoint(const Coordinate& coordinate) const;

	/// Creates a Point taking ownership of the given CoordinateSequence
	Point* createPoint(CoordinateSequence *coordinates) const;

	/// Creates a Point with a deep-copy of the given CoordinateSequence.
	Point* createPoint(const CoordinateSequence &coordinates) const;

	/// Construct an EMPTY GeometryCollection
	GeometryCollection* createGeometryCollection() const;

	/// Construct a GeometryCollection taking ownership of given arguments
	GeometryCollection* createGeometryCollection(vector<Geometry *> *newGeoms) const;

	/// Constructs a GeometryCollection with a deep-copy of args
	GeometryCollection* createGeometryCollection(const vector<Geometry *> &newGeoms) const;

	/// Construct an EMPTY MultiLineString 
	MultiLineString* createMultiLineString() const;

	/// Construct a MultiLineString taking ownership of given arguments
	MultiLineString* createMultiLineString(vector<Geometry *> *newLines) const;

	/// Construct a MultiLineString with a deep-copy of given arguments
	MultiLineString* createMultiLineString(const vector<Geometry *> &fromLines) const;

	/// Construct an EMPTY MultiPolygon 
	MultiPolygon* createMultiPolygon() const;

	/// Construct a MultiPolygon taking ownership of given arguments
	MultiPolygon* createMultiPolygon(vector<Geometry *> *newPolys) const;

	/// Construct a MultiPolygon with a deep-copy of given arguments
	MultiPolygon* createMultiPolygon(const vector<Geometry *> &fromPolys) const;

	/// Construct an EMPTY LinearRing 
	LinearRing* createLinearRing() const;

	/// Construct a LinearRing taking ownership of given arguments
	LinearRing* createLinearRing(CoordinateSequence* newCoords) const;

	/// Construct a LinearRing with a deep-copy of given arguments
	LinearRing* createLinearRing(const CoordinateSequence& coordinates) const;

	/// Constructs an EMPTY <code>MultiPoint</code>.
	MultiPoint* createMultiPoint() const;

	/// Construct a MultiPoint taking ownership of given arguments
	MultiPoint* createMultiPoint(vector<Geometry *> *newPoints) const;

	/// Construct a MultiPoint with a deep-copy of given arguments
	MultiPoint* createMultiPoint(const vector<Geometry *> &fromPoints) const;

	/// Construct a MultiPoint containing a Point geometry for each Coordinate in the given list.
	MultiPoint* createMultiPoint(const CoordinateSequence &fromCoords) const;

	/// Construct an EMPTY Polygon 
	Polygon* createPolygon() const;

	/// Construct a Polygon taking ownership of given arguments
	Polygon* createPolygon(LinearRing *shell, vector<Geometry *> *holes) const;

	/// Construct a Polygon with a deep-copy of given arguments
	Polygon* createPolygon(const LinearRing &shell, const vector<Geometry *> &holes) const;

	/// Construct an EMPTY LineString 
	LineString* createLineString() const;

	/// Construct a LineString taking ownership of given argument
	LineString* createLineString(CoordinateSequence* coordinates) const;

	/// Construct a LineString with a deep-copy of given argument
	LineString* createLineString(const CoordinateSequence& coordinates) const;

	/// Construct the most suitable Geometry using the given Geometries; will take ownership of arguments.
	Geometry* buildGeometry(vector<Geometry *> *geoms) const;

	/// Construct the most suitable Geometry using the given Geometries; will use a deep-copy of arguments.
	Geometry* buildGeometry(const vector<Geometry *> &geoms) const;
	
	int getSRID() const {return SRID;};

	const CoordinateSequenceFactory* getCoordinateSequenceFactory() const {return coordinateListFactory;};

	/// Returns a clone of given Geometry.
	Geometry* createGeometry(const Geometry *g) const;

	/// Destroy a Geometry, or release it
	void destroyGeometry(Geometry *g) const;

private:
	const PrecisionModel* precisionModel;
	int SRID;
#ifdef INT64_CONST_IS_I64
	static const int64 serialVersionUID = -6820524753094095635I64;
#else
	static const int64 serialVersionUID = -6820524753094095635LL;
#endif        
	const CoordinateSequenceFactory *coordinateListFactory;
};

/*
 * Represents a planar triangle, and provides methods for calculating various
 * properties of triangles.
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

} // namespace geos
#endif

/**********************************************************************
 * $Log$
 * Revision 1.42  2005/04/29 17:40:36  strk
 * Updated Doxygen documentation and some Copyright headers.
 *
 * Revision 1.41  2005/04/29 11:52:40  strk
 * Added new JTS interfaces for CoordinateSequence and factories,
 * removed example implementations to reduce maintainance costs.
 * Added first implementation of WKBWriter, made ByteOrderDataInStream
 * a template class.
 *
 * Revision 1.40  2005/04/20 17:22:46  strk
 * Added initial implementation of WKBReaderT and ByteOrderDataInStreamT
 * class templates and ByteOrderValues class.
 * Work is unfinished as WKBReader requires new interface of CoordinateSequence
 * taking higher dimensions into account.
 *
 * Revision 1.39  2005/02/17 09:56:03  strk
 * Applied patch from Jon Schlueter using <cmath> instead of <math.h>
 *
 * Revision 1.38  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.37  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.36  2005/02/04 23:55:23  strk
 * Envelope destructor made non-virtual to give compiler more static
 * binding options.
 *
 * Revision 1.35  2004/12/16 16:27:24  strk
 * Fixed LinearRing::clone() to return LinearRing instead of LineString
 *
 * Revision 1.34  2004/12/03 22:52:56  strk
 * enforced const return of CoordinateSequence::toVector() method to derivate classes.
 *
 * Revision 1.33  2004/12/03 16:22:36  frank
 * update to use I64 on MSVC for 64 bit integer constants, also toVector chg.
 *
 * Revision 1.32  2004/11/29 16:05:33  strk
 * Fixed a bug in LineIntersector::interpolateZ causing NaN values
 * to come out.
 * Handled dimensional collapses in ElevationMatrix.
 * Added ISNAN macro and changed ISNAN/FINITE macros to avoid
 * dispendious isnan() and finite() calls.
 *
 * Revision 1.31  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.30  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.29  2004/11/08 10:57:42  strk
 * Moved Log lines at the bottom, and cut oldest
 *
 * Revision 1.28  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.27  2004/09/16 09:48:06  strk
 * Added Envelope::equals
 *
 * Revision 1.26  2004/09/13 12:39:14  strk
 * Made Point and MultiPoint subject to Validity tests.
 *
 * Revision 1.25  2004/09/07 08:29:57  strk
 * Fixed doxygen malformed comment for Coordinate class
 *
 * Revision 1.24  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.23  2004/07/26 16:35:19  strk
 * Removed dangling MultiPoint::isClosed() method definition.
 *
 * Revision 1.22  2004/07/22 16:58:01  strk
 * runtime version extractor functions split. geos::version() is now
 * geos::geosversion() and geos::jtsport()
 *
 **********************************************************************/
