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
 * Revision 1.2  2004/07/05 10:50:21  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateList
 * Changed createMultiPoint(CoordinateList) signature to reflect
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


/*
* $Log$
* Revision 1.2  2004/07/05 10:50:21  strk
* deep-dopy construction taken out of Geometry and implemented only
* in GeometryFactory.
* Deep-copy geometry construction takes care of cleaning up copies
* on exception.
* Implemented clone() method for CoordinateList
* Changed createMultiPoint(CoordinateList) signature to reflect
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
* Revision 1.11  2003/10/23 09:12:49  strk
* Made CoordinateArrayFilter destructor virtual.
*
* Revision 1.10  2003/10/16 17:05:07  strk
* Made TopologyException inherit from GEOSException. Adjusted IllegalArgumentException subclassing.
*
*/
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
 * Base class for all GEOS exceptions.
 * Exceptions are thrown as pointers to this type.
 * Use toString() to get a readable message.
 */
class GEOSException {
public:
	GEOSException();
	GEOSException(string msg);
	GEOSException(string nname,string msg);
	virtual ~GEOSException();
	virtual string toString();
	virtual void setName(string nname);
	virtual void setMessage(string msg);
protected:
	string txt;
	string name;
};

class AssertionFailedException: public GEOSException {
public:
	AssertionFailedException();
	AssertionFailedException(string msg);
	~AssertionFailedException();
};

class IllegalArgumentException: public GEOSException {
public:
	IllegalArgumentException();
	IllegalArgumentException(string msg);
	~IllegalArgumentException();
};

/**
 * Indicates an invalid or inconsistent topological situation encountered during processing
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
	CoordinateList* pts;
	int n;
	CoordinateArrayFilter(int size);
	virtual ~CoordinateArrayFilter();
	virtual const CoordinateList* getCoordinates() const;
	virtual void filter_ro(const Coordinate &coord);
	virtual void filter_rw(Coordinate &coord); // Unsopported
};

class UniqueCoordinateArrayFilter:public CoordinateFilter {
public:
	CoordinateList *list;
	UniqueCoordinateArrayFilter();
	virtual ~UniqueCoordinateArrayFilter();
	virtual const CoordinateList* getCoordinates() const;
	virtual void filter_ro(const Coordinate *coord);
	virtual void filter_rw(Coordinate *coord); // Unsupported
};


/**
 * Computes various kinds of common geometric shapes.
 * Allows various ways of specifying the location and extent of the shapes,
 * as well as number of line segments used to form them.
 *
 */
class GeometricShapeFactory {
private:
	class Dimensions {
	public:
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
	GeometryFactory* geomFact;
	Dimensions* dim;
	int nPts;
public:
	/**
	* Create a shape factory which will create shapes using the given
	* {@link GeometryFactory}.
	*
	* @param geomFact the factory to use
	*/
	GeometricShapeFactory(GeometryFactory *newGeomFact);
	~GeometricShapeFactory();
	/**
	* Sets the location of the shape by specifying the base coordinate
	* (which in most cases is the
	* lower left point of the envelope containing the shape).
	*
	* @param base the base coordinate of the shape
	*/
	void setBase(const Coordinate& base);
	/**
	* Sets the location of the shape by specifying the centre of
	* the shape's bounding box
	*
	* @param centre the centre coordinate of the shape
	*/
	void setCentre(const Coordinate& centre);
	/**
	* Sets the total number of points in the created Geometry
	*/
	void setNumPoints(int nNPts);
	/**
	* Sets the size of the extent of the shape in both x and y directions.
	*
	* @param size the size of the shape's extent
	*/
	void setSize(double size);
	/**
	* Sets the width of the shape.
	*
	* @param width the width of the shape
	*/
	void setWidth(double width);
	/**
	* Sets the height of the shape.
	*
	* @param height the height of the shape
	*/
	void setHeight(double height);
	/**
	* Creates a rectangular {@link Polygon}.
	*
	* @return a rectangular Polygon
	*
	*/
	Polygon* createRectangle();
	/**
	* Creates a circular {@link Polygon}.
	*
	* @return a circle
	*/
	Polygon* createCircle();
	/**
	* Creates a elliptical arc, as a LineString.
	*
	* @return an elliptical arc
	*/
	LineString* createArc(double startAng,double endAng);
};

}
#endif
