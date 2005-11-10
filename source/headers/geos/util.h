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
 **********************************************************************/

#ifndef GEOS_UTIL_H
#define GEOS_UTIL_H

#include <memory>
#include <string>
#include <geos/platform.h>
#include <geos/geom.h>

using namespace std;

namespace geos {

/**
 * \class GEOSException util.h geos.h
 *
 * \brief Base class for all GEOS exceptions.
 *
 * Exceptions are thrown as pointers to this type.
 * Use toString() to get a readable message.
 */
class GEOSException {
public:
	GEOSException();

	GEOSException(string msg);

	/// Create an exception of given type containing given message 
	GEOSException(string nname,string msg);

	virtual ~GEOSException();

	/// Returns exception message
	virtual string toString();

	virtual void setName(string nname);
	virtual void setMessage(string msg);
protected:
	string txt;
	string name;
};

/** \class AssertionFailedException util.h geos.h
 * \brief Indicates a bug in GEOS code.
 */
class AssertionFailedException: public GEOSException {
public:
	AssertionFailedException();
	AssertionFailedException(string msg);
	~AssertionFailedException();
};

/** \class IllegalArgumentException util.h geos.h
 * \brief Indicates one or more legal arguments.
 *
 * This exception is thrown - for example - when
 * trying to apply set-theoretic methods to a
 * GeometryCollection object.
 */
class IllegalArgumentException: public GEOSException {
public:
	IllegalArgumentException();
	IllegalArgumentException(string msg);
	~IllegalArgumentException();
};

/**
 * \class TopologyException util.h geos.h
 *
 * \brief
 * Indicates an invalid or inconsistent topological situation encountered
 * during processing
 */
class TopologyException: public GEOSException {
public:
	TopologyException(string msg);
	TopologyException(string msg,const Coordinate *newPt);
	~TopologyException();
	Coordinate* getCoordinate();
private:
	Coordinate *pt;
};

/**
 * \class UnsupportedOperationException util.h geos.h
 *
 * \brief Indicates that the requested operation is unsopported.
 *
 * This exception is thrown - for example - when requesting the
 * X or Y member of an empty Point
 */
class UnsupportedOperationException: public GEOSException {
public:
	UnsupportedOperationException();
	UnsupportedOperationException(string msg);
	~UnsupportedOperationException();
};

class Coordinate;
class Assert {
public:
	static void isTrue(bool assertion);
	static void isTrue(bool assertion, string message);

	static void equals(const Coordinate& expectedValue, const Coordinate& actualValue);
	static void equals(const Coordinate& expectedValue, const Coordinate& actualValue, string message);

	static void shouldNeverReachHere();
	static void shouldNeverReachHere(string message);
};

class CoordinateArrayFilter:public CoordinateFilter {
public:
	CoordinateSequence* pts;
	int n;
	CoordinateArrayFilter(int size);
	virtual ~CoordinateArrayFilter();
	virtual const CoordinateSequence* getCoordinates() const;
	virtual void filter_ro(const Coordinate *coord);
	virtual void filter_rw(Coordinate *coord); // Unsopported
};

class UniqueCoordinateArrayFilter:public CoordinateFilter {
public:
	CoordinateSequence *list;
	UniqueCoordinateArrayFilter();
	virtual ~UniqueCoordinateArrayFilter();
	virtual const CoordinateSequence* getCoordinates() const;
	virtual void filter_ro(const Coordinate *coord);
	virtual void filter_rw(Coordinate *coord); // Unsupported
};


/**
 * \class GeometricShapeFactory util.h geos.h
 *
 * \brief
 * Computes various kinds of common geometric shapes.
 * Allows various ways of specifying the location and extent of the shapes,
 * as well as number of line segments used to form them.
 *
 */
class GeometricShapeFactory {
private:
	class Dimensions {
	public:
		Dimensions();
		Coordinate base;
		Coordinate centre;
		double width;
		double height;
		void setBase(const Coordinate& newBase);
		void setCentre(const Coordinate& newCentre);
		void setSize(double size);
		void setWidth(double nWidth);
		void setHeight(double nHeight);
		Envelope* getEnvelope();
	};
	const GeometryFactory* geomFact;
	Dimensions dim;
	int nPts;
public:
	/**
	* \brief
	* Create a shape factory which will create shapes using the given
	* GeometryFactory.
	*
	* @param factory the factory to use. You need to keep the
	*	factory alive for the whole GeometricShapeFactory
	*	life time.
	* 
	*/
	GeometricShapeFactory(const GeometryFactory *factory);

	~GeometricShapeFactory();

	/**
	* \brief Creates a elliptical arc, as a LineString.
	*
	* @return an elliptical arc
	*/
	LineString* createArc(double startAng,double endAng);

	/**
	* \brief Creates a circular Polygon.
	*
	* @return a circle
	*/
	Polygon* createCircle();

	/**
	* \brief Creates a rectangular Polygon.
	*
	* @return a rectangular Polygon
	*/
	Polygon* createRectangle();

	/**
	* \brief
	* Sets the location of the shape by specifying the base coordinate
	* (which in most cases is the * lower left point of the envelope
	* containing the shape).
	*
	* @param base the base coordinate of the shape
	*/
	void setBase(const Coordinate& base);

	/**
	* \brief
	* Sets the location of the shape by specifying the centre of
	* the shape's bounding box
	*
	* @param centre the centre coordinate of the shape
	*/
	void setCentre(const Coordinate& centre);

	/**
	* \brief Sets the height of the shape.
	*
	* @param height the height of the shape
	*/
	void setHeight(double height);

	/**
	* \brief Sets the total number of points in the created Geometry
	*/
	void setNumPoints(int nNPts);

	/**
	* \brief
	* Sets the size of the extent of the shape in both x and y directions.
	*
	* @param size the size of the shape's extent
	*/
	void setSize(double size);

	/**
	* \brief Sets the width of the shape.
	*
	* @param width the width of the shape
	*/
	void setWidth(double width);

};

}
#endif

/**********************************************************************
 * $Log$
 * Revision 1.9  2005/11/10 10:24:08  strk
 * Fixed virtual overload of CoordinateArrayFilter::filter*
 *
 * Revision 1.8  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.7  2004/07/16 10:28:41  strk
 * Dimesions object allocated on the heap
 *
 * Revision 1.6  2004/07/14 21:19:35  strk
 * GeometricShapeFactory first pass of bug fixes
 *
 * Revision 1.5  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.4  2004/07/07 10:29:54  strk
 * Adjusted exceptions documentation.
 *
 * Revision 1.3  2004/07/05 14:23:03  strk
 * More documentation cleanups.
 *
 * Revision 1.2  2004/07/05 10:50:21  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.14  2004/04/10 22:41:25  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 * Revision 1.13  2004/03/18 10:42:44  ybychkov
 * "IO" and "Util" upgraded to JTS 1.4
 * "Geometry" partially upgraded.
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

