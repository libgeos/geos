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

#include <geos/geom.h>
#include <geos/util.h>

namespace geos {


/**
 * Creates a Point using the given CoordinateSequence (must have 1 element)
 *
 * @param  newCoords
 *	contains the single coordinate on which to base this
 *	<code>Point</code> or <code>null</code> to create
 *	the empty geometry.
 *
 *	If not null the created Point will take ownership of newCoords.
 */  
Point::Point(CoordinateSequence *newCoords, const GeometryFactory *factory):
	Geometry(factory)
{
	if (newCoords==NULL) {
		coordinates=factory->getCoordinateSequenceFactory()->create(NULL);
		return;
	}        
	if (newCoords->getSize() != 1)
	{
		throw new IllegalArgumentException("Point coordinate list must contain a single element");
	}
	coordinates=newCoords;
}

Point::Point(const Point &p): Geometry(p.getFactory())
{
	coordinates=p.coordinates->clone();
}

CoordinateSequence *
Point::getCoordinates() const
{
	return coordinates->clone();
}

int
Point::getNumPoints() const
{
	return isEmpty() ? 0 : 1;
}

bool
Point::isEmpty() const
{
	return coordinates->isEmpty();
}

bool
Point::isSimple() const
{
	return true;
}

//bool Point::isValid() const {return true;}

int
Point::getDimension() const
{
	return 0;
}

int
Point::getBoundaryDimension() const
{
	return Dimension::False;
}

double
Point::getX() const
{
	if (isEmpty()) {
		throw new UnsupportedOperationException("getX called on empty Point\n");
	}
	return getCoordinate()->x;
}

double
Point::getY() const
{
	if (isEmpty()) {
		throw new UnsupportedOperationException("getY called on empty Point\n");
	}
	return getCoordinate()->y;
}

const Coordinate *
Point::getCoordinate() const
{
	return coordinates->getSize()!=0 ? &(coordinates->getAt(0)) : NULL;
}

string
Point::getGeometryType() const
{
	return "Point";
}

Geometry *
Point::getBoundary() const
{
	return getFactory()->createGeometryCollection(NULL);
}

Envelope *
Point::computeEnvelopeInternal() const
{
	if (isEmpty()) {
		return new Envelope();
	}
	return new Envelope(getCoordinate()->x, getCoordinate()->x, getCoordinate()->y, getCoordinate()->y);
}

void
Point::apply_ro(CoordinateFilter *filter) const
{
	if (isEmpty()) {return;}
	filter->filter_ro(getCoordinate());
}

void
Point::apply_rw(CoordinateFilter *filter)
{
	if (isEmpty()) {return;}
	Coordinate newcoord = coordinates->getAt(0);
	filter->filter_rw(&newcoord);
	coordinates->setAt(newcoord, 0);
}

void
Point::apply_rw(GeometryFilter *filter)
{
	filter->filter_rw(this);
}

void
Point::apply_ro(GeometryFilter *filter) const
{
	filter->filter_ro(this);
}

void
Point::apply_rw(GeometryComponentFilter *filter)
{
	filter->filter_rw(this);
}

void
Point::apply_ro(GeometryComponentFilter *filter) const
{
	filter->filter_ro(this);
}

bool
Point::equalsExact(const Geometry *other, double tolerance) const
{
	if (!isEquivalentClass(other)) {
		return false;
	}
	if (isEmpty() && other->isEmpty()) {
		return true;
	}
	bool ret = equal(*((Point*) other)->getCoordinate(), *getCoordinate(),
		tolerance);
	return ret;
}

int
Point::compareToSameClass(const Geometry *point) const
{
	return getCoordinate()->compareTo(*(((Point*)point)->getCoordinate()));
}

Point::~Point()
{
	delete coordinates;
}

GeometryTypeId
Point::getGeometryTypeId() const
{
	return GEOS_POINT;
}

} // namesapce geos

/**********************************************************************
 *
 * $Log$
 * Revision 1.35  2005/06/23 14:22:33  strk
 * Inlined and added missing ::clone() for Geometry subclasses
 *
 * Revision 1.34  2005/05/23 16:42:43  strk
 * Added Refractions copyright
 *
 * Revision 1.33  2005/05/13 17:15:34  strk
 * cleanups and indentations
 *
 * Revision 1.32  2005/04/19 11:49:26  strk
 * Fixed segfault in ::isEmpty
 *
 * Revision 1.31  2004/11/23 16:22:49  strk
 * Added ElevationMatrix class and components to do post-processing draping of overlayed geometries.
 *
 * Revision 1.30  2004/09/13 12:39:14  strk
 * Made Point and MultiPoint subject to Validity tests.
 *
 * Revision 1.29  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 **********************************************************************/

