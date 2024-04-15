/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences LLC
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/Polygon.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/geom/Curve.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Surface.h>
#include <geos/util.h>

namespace geos {
namespace geom {

void
Surface::apply_ro(CoordinateFilter* filter) const
{
    getExteriorRing()->apply_ro(filter);
    for (std::size_t i = 0; i < getNumInteriorRing(); i++) {
        getInteriorRingN(i)->apply_ro(filter);
    }
}

void
Surface::apply_ro(CoordinateSequenceFilter& filter) const
{
    getExteriorRing()->apply_ro(filter);

    for (std::size_t i = 0; !filter.isDone() && i < getNumInteriorRing(); i++) {
        getInteriorRingN(i)->apply_ro(filter);
    }
}

void
Surface::apply_ro(GeometryComponentFilter* filter) const
{
    filter->filter_ro(this);
    getExteriorRing()->apply_ro(filter);
    for (std::size_t i = 0; !filter->isDone() && i < getNumInteriorRing(); i++) {
        getInteriorRingN(i)->apply_ro(filter);
    }
}

void
Surface::apply_ro(GeometryFilter* filter) const
{
    filter->filter_ro(this);
}

void
Surface::apply_rw(const CoordinateFilter* filter)
{
    getExteriorRing()->apply_rw(filter);
    for (std::size_t i = 0; i < getNumInteriorRing(); i++) {
        getInteriorRingN(i)->apply_rw(filter);
    }
}

void
Surface::apply_rw(CoordinateSequenceFilter& filter)
{
    getExteriorRing()->apply_rw(filter);

    for (std::size_t i = 0; !filter.isDone() && i < getNumInteriorRing(); i++) {
        getInteriorRingN(i)->apply_rw(filter);
    }

    if (filter.isGeometryChanged()) {
        geometryChanged();
    }
}

void
Surface::apply_rw(GeometryComponentFilter* filter)
{
    filter->filter_rw(this);
    getExteriorRing()->apply_rw(filter);
    for (std::size_t i = 0; !filter->isDone() && i < getNumInteriorRing(); i++) {
        getInteriorRingN(i)->apply_rw(filter);
    }
}

void
Surface::apply_rw(GeometryFilter* filter)
{
    filter->filter_rw(this);
}

int
Surface::compareToSameClass(const Geometry* g) const
{
    const Surface* p = detail::down_cast<const Surface*>(g);
    int shellComp = getExteriorRing()->compareTo(p->getExteriorRing());
    if (shellComp != 0) {
        return shellComp;
    }

    size_t nHole1 = getNumInteriorRing();
    size_t nHole2 = p->getNumInteriorRing();
    if (nHole1 < nHole2) {
        return -1;
    }
    if (nHole1 > nHole2) {
        return 1;
    }

    for (size_t i=0; i < nHole1; i++) {
        const Curve* lr = p->getInteriorRingN(i);
        const int holeComp = getInteriorRingN(i)->compareTo(lr);
        if (holeComp != 0) {
            return holeComp;
        }
    }

    return 0;
}

std::unique_ptr<Geometry>
Surface::convexHull() const
{
    return getExteriorRing()->convexHull();
}

std::unique_ptr<Geometry>
Surface::createEmptyRing(const GeometryFactory& factory)
{
    return factory.createLinearRing();
}

bool
Surface::equalsExact(const Geometry* other, double tolerance) const
{
    if (!isEquivalentClass(other)) {
        return false;
    }

    const Surface* otherPolygon = detail::down_cast<const Surface*>(other);
    if (! otherPolygon) {
        return false;
    }

    if (!getExteriorRing()->equalsExact(otherPolygon->getExteriorRing(), tolerance)) {
        return false;
    }

    if (getNumInteriorRing() != otherPolygon->getNumInteriorRing()) {
        return false;
    }

    for (std::size_t i = 0; i < getNumInteriorRing(); i++) {
        const Curve* hole = getInteriorRingN(i);
        const Curve* otherhole = otherPolygon->getInteriorRingN(i);
        if (!hole->equalsExact(otherhole, tolerance)) {
            return false;
        }
    }

    return true;
}

bool
Surface::equalsIdentical(const Geometry* other_g) const
{
    if (!isEquivalentClass(other_g)) {
        return false;
    }

    const auto& other = static_cast<const Surface&>(*other_g);

    if (getNumInteriorRing() != other.getNumInteriorRing()) {
        return false;
    }

    if (!getExteriorRing()->equalsIdentical(other.getExteriorRing())) {
        return false;
    }

    for (std::size_t i = 0; i < getNumInteriorRing(); i++) {
        if (!getInteriorRingN(i)->equalsIdentical(other.getInteriorRingN(i))) {
            return false;
        }
    }

    return true;
}

const CoordinateXY*
Surface::getCoordinate() const
{
    return getExteriorRing()->getCoordinate();
}

uint8_t
Surface::getCoordinateDimension() const
{
    uint8_t dimension = 2;

    if (getExteriorRing() != nullptr) {
        dimension = std::max(dimension, getExteriorRing()->getCoordinateDimension());
    }

    for (std::size_t i = 0; i < getNumInteriorRing(); i++) {
        dimension = std::max(dimension, getInteriorRingN(i)->getCoordinateDimension());
    }

    return dimension;
}

const Envelope*
Surface::getEnvelopeInternal() const
{
    return getExteriorRing()->getEnvelopeInternal();
}

double
Surface::getLength() const
{
    double len = 0.0;
    len += getExteriorRing()->getLength();
    for (std::size_t i = 0; i < getNumInteriorRing(); i++) {
        len += getInteriorRingN(i)->getLength();
    }
    return len;
}

size_t
Surface::getNumPoints() const
{
    std::size_t numPoints = getExteriorRing()->getNumPoints();
    for (std::size_t i = 0; i < getNumInteriorRing(); i++) {
        numPoints += getInteriorRingN(i)->getNumPoints();
    }
    return numPoints;
}

bool
Surface::hasM() const
{
    if (getExteriorRing()->hasM()) {
        return true;
    }
    for (std::size_t i = 0 ; i < getNumInteriorRing(); i++) {
        if (getInteriorRingN(i)->hasM()) {
            return true;
        }
    }
    return false;
}

bool
Surface::hasZ() const
{
    if (getExteriorRing()->hasZ()) {
        return true;
    }
    for (std::size_t i = 0 ; i < getNumInteriorRing(); i++) {
        if (getInteriorRingN(i)->hasZ()) {
            return true;
        }
    }
    return false;
}

bool
Surface::isEmpty() const
{
    return getExteriorRing()->isEmpty();
}

}
}
