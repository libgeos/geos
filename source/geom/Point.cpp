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
 * Revision 1.27  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateList
 * Changed createMultiPoint(CoordinateList) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.26  2004/07/02 14:27:32  strk
 * Added deep-copy / take-ownerhship for Point type.
 *
 * Revision 1.25  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.24  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.23  2004/06/28 21:11:43  strk
 * Moved getGeometryTypeId() definitions from geom.h to each geometry module.
 * Added holes argument check in Polygon.cpp.
 *
 * Revision 1.22  2004/04/16 14:12:52  strk
 * Memory leak fix in copy constructor
 *
 * Revision 1.21  2004/04/16 08:35:52  strk
 * Memory leaks fixed and const correctness applied for Point class.
 *
 * Revision 1.20  2004/04/13 14:45:54  strk
 * Removed faulty assert in constructor
 *
 * Revision 1.19  2004/04/10 22:41:24  ybychkov
 * "precision" upgraded to JTS 1.4
 *
 * Revision 1.18  2004/03/31 07:50:37  ybychkov
 * "geom" partially upgraded to JTS 1.4
 *
 * Revision 1.17  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geom.h>
#include <geos/util.h>

namespace geos {

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
Point::Point(const Coordinate& c, const PrecisionModel* precisionModel,
			 int SRID): Geometry(new GeometryFactory(precisionModel,SRID,CoordinateListFactory::internalFactory)) {
	coordinates=CoordinateListFactory::internalFactory->createCoordinateList();
	if (!(c==Coordinate::nullCoord)) {
		coordinates->add(c);
	}
}

/**
* Creates a Point using the given CoordinateList (must have 1 element)
*
* @param  newCoords
*	contains the single coordinate on which to base this
*	<code>Point</code> or <code>null</code> to create
*	the empty geometry.
*
*	If not null the created Point will take ownership of newCoords.
*/  
Point::Point(CoordinateList *newCoords, const GeometryFactory *factory): Geometry(factory) {
	if (newCoords==NULL) {
		coordinates=CoordinateListFactory::internalFactory->createCoordinateList();
		return;
	}        
	if (newCoords->getSize() != 1)
	{
		throw new IllegalArgumentException("Point coordinate list must contain a single element");
	}
	coordinates=newCoords;
}

Point::Point(const Point &p): Geometry(p.getFactory()) {
	coordinates=CoordinateListFactory::internalFactory->createCoordinateList(p.coordinates);;
}

Geometry* Point::clone() const {
	return new Point(*this);
}

CoordinateList* Point::getCoordinates() const {
	if (isEmpty()) {
		return CoordinateListFactory::internalFactory->createCoordinateList();
	} else {
		return CoordinateListFactory::internalFactory->createCoordinateList(coordinates);
	}
}
int Point::getNumPoints() const {
	return isEmpty() ? 0 : 1;
}

bool Point::isEmpty() const {
	return (*getCoordinate())==Coordinate::getNull();
}

bool Point::isSimple() const {return true;}
bool Point::isValid() const {return true;}
int Point::getDimension() const {return 0;}
int Point::getBoundaryDimension() const {return Dimension::False;}

double Point::getX() const {
	if (isEmpty()) {
		throw new UnsupportedOperationException("getX called on empty Point\n");
	}
	return getCoordinate()->x;
}

double Point::getY() const {
	if (isEmpty()) {
		throw new UnsupportedOperationException("getY called on empty Point\n");
	}
	return getCoordinate()->y;
}

const Coordinate* Point::getCoordinate() const {
	return coordinates->getSize()!=0 ? &(coordinates->getAt(0)) : NULL;
}

string Point::getGeometryType() const {
	return "Point";
}

Geometry* Point::getBoundary() const {
	return getFactory()->createGeometryCollection(NULL);
}

Envelope* Point::computeEnvelopeInternal() const {
	if (isEmpty()) {
		return new Envelope();
	}
	return new Envelope(getCoordinate()->x, getCoordinate()->x, getCoordinate()->y, getCoordinate()->y);
}

void Point::apply_ro(CoordinateFilter *filter) const {
    if (isEmpty()) {return;}
	filter->filter_ro(getCoordinate());
}

void Point::apply_rw(CoordinateFilter *filter) {
}

void Point::apply_rw(GeometryFilter *filter) {
	filter->filter_rw(this);
}

void Point::apply_ro(GeometryFilter *filter) const {
	filter->filter_ro(this);
}

void Point::apply_rw(GeometryComponentFilter *filter) {
	filter->filter_rw(this);
}

void Point::apply_ro(GeometryComponentFilter *filter) const {
	filter->filter_ro(this);
}

bool Point::equalsExact(const Geometry *other, double tolerance) const
{
	if (!isEquivalentClass(other)) {
		return false;
	}
	if (isEmpty() && other->isEmpty()) {
		return true;
	}
    return equal(*((Point*) other)->getCoordinate(), *getCoordinate(), tolerance);
}

int Point::compareToSameClass(const Geometry *point) const {
	return getCoordinate()->compareTo(*(((Point*)point)->getCoordinate()));
}

Point::~Point(){
	delete coordinates;
}
GeometryTypeId
Point::getGeometryTypeId() const {
	return GEOS_POINT;
}
}

