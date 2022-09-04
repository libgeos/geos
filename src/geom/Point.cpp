/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/Point.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/util/UnsupportedOperationException.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util.h>

#include <string>
#include <memory>

namespace geos {
namespace geom { // geos::geom



/*protected*/
Point::Point(CoordinateSequence* newCoords, const GeometryFactory* factory)
    : Geometry(factory)
{
    std::unique_ptr<CoordinateSequence> coords(newCoords); // Take ownership in case of exception

    if (coords->getSize() <= 1) {
        coordinates = *(coords.get());
    } else if (coords->getSize() > 1) {
        throw util::IllegalArgumentException("Point coordinate list must contain a single element");
    }
}

Point::Point(const Coordinate & c, const GeometryFactory* factory)
    : Geometry(factory),
    coordinates(1)
{
    coordinates.setAt(c, 0);
}

Point::Point(const CoordinateXY & c, const GeometryFactory* factory)
    : Geometry(factory),
      coordinates(1)
{
    coordinates.setAt(Coordinate(c), 0);
}

/*protected*/
Point::Point(const Point& p)
    : Geometry(p)
    , coordinates(p.coordinates)
{}

std::unique_ptr<CoordinateSequence>
Point::getCoordinates() const
{
    return getCoordinatesRO()->clone();
}

std::size_t
Point::getNumPoints() const
{
    return coordinates.size();
}

bool
Point::isEmpty() const
{
    if (coordinates.isEmpty()) {
        return true;
    }

    const Coordinate& c = coordinates.getAt(0);
    if (std::isnan(c.x) && std::isnan(c.y)) {
        return true;
    }

    return false;
}

bool
Point::isSimple() const
{
    return true;
}

Dimension::DimensionType
Point::getDimension() const
{
    return Dimension::P; // point
}

uint8_t
Point::getCoordinateDimension() const
{
    return (uint8_t) getCoordinatesRO()->getDimension();
}

int
Point::getBoundaryDimension() const
{
    return Dimension::False;
}

double
Point::getX() const
{
    if(isEmpty()) {
        throw util::UnsupportedOperationException("getX called on empty Point\n");
    }
    return getCoordinate()->x;
}

double
Point::getY() const
{
    if(isEmpty()) {
        throw util::UnsupportedOperationException("getY called on empty Point\n");
    }
    return getCoordinate()->y;
}

double
Point::getZ() const
{
    if(isEmpty()) {
        throw util::UnsupportedOperationException("getZ called on empty Point\n");
    }
    return getCoordinate()->z;
}

std::string
Point::getGeometryType() const
{
    return "Point";
}

std::unique_ptr<Geometry>
Point::getBoundary() const
{
    return getFactory()->createGeometryCollection();
}

Envelope::Ptr
Point::computeEnvelopeInternal() const
{
    if(isEmpty()) {
        return Envelope::Ptr(new Envelope());
    }

    return Envelope::Ptr(new Envelope(getCoordinate()->x,
                                      getCoordinate()->x, getCoordinate()->y,
                                      getCoordinate()->y));
}

void
Point::apply_ro(CoordinateFilter* filter) const
{
    if(isEmpty()) {
        return;
    }
    filter->filter_ro(getCoordinate());
}

void
Point::apply_rw(const CoordinateFilter* filter)
{
    if (isEmpty()) {
        return;
    }
    coordinates.apply_rw(filter);
}

void
Point::apply_rw(GeometryFilter* filter)
{
    filter->filter_rw(this);
}

void
Point::apply_ro(GeometryFilter* filter) const
{
    filter->filter_ro(this);
}

void
Point::apply_rw(GeometryComponentFilter* filter)
{
    filter->filter_rw(this);
}

void
Point::apply_ro(GeometryComponentFilter* filter) const
{
    filter->filter_ro(this);
}

void
Point::apply_rw(CoordinateSequenceFilter& filter)
{
    if(isEmpty()) {
        return;
    }
    filter.filter_rw(coordinates, 0);
    if(filter.isGeometryChanged()) {
        geometryChanged();
    }
}

void
Point::apply_ro(CoordinateSequenceFilter& filter) const
{
    if(isEmpty()) {
        return;
    }
    filter.filter_ro(coordinates, 0);
    //if (filter.isGeometryChanged()) geometryChanged();
}

bool
Point::equalsExact(const Geometry* other, double tolerance) const
{
    if(!isEquivalentClass(other)) {
        return false;
    }

    // assume the isEquivalentClass would return false
    // if other is not a point
    assert(dynamic_cast<const Point*>(other));

    if(isEmpty()) {
        return other->isEmpty();
    }
    else if(other->isEmpty()) {
        return false;
    }

    const CoordinateXY* this_coord = getCoordinate();
    const CoordinateXY* other_coord = other->getCoordinate();

    // assume the isEmpty checks above worked :)
    assert(this_coord && other_coord);

    return equal(*this_coord, *other_coord, tolerance);
}

int
Point::compareToSameClass(const Geometry* g) const
{
    const Point* p = detail::down_cast<const Point*>(g);
    return getCoordinate()->compareTo(*(p->getCoordinate()));
}

GeometryTypeId
Point::getGeometryTypeId() const
{
    return GEOS_POINT;
}

/*public*/
const CoordinateSequence*
Point::getCoordinatesRO() const
{
    return &coordinates;
}

} // namespace geos::geom
} // namesapce geos
