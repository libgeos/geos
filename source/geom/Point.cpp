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


#include "../headers/geom.h"
#include "../headers/util.h"

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
*@param  coordinates      contains the single coordinate on which to base this <code>Point</code>
*      , or <code>null</code> to create the empty geometry.
*/  
Point::Point(CoordinateList *newCoordinates, GeometryFactory *newFactory): Geometry(newFactory) {
	if (newCoordinates==NULL) {
		newCoordinates=CoordinateListFactory::internalFactory->createCoordinateList();
	}        
	Assert::isTrue(coordinates->getSize()<=1);
	coordinates=newCoordinates;
}

Point::Point(const Point &p): Geometry(p.getFactory()) {
	coordinates=CoordinateListFactory::internalFactory->createCoordinateList(p.getCoordinates());;
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
}
}

