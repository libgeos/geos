/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
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

#include <geos/algorithm/Area.h>
#include <geos/algorithm/Orientation.h>
#include <geos/util.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h> // for getBoundary()
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/util.h>

#include <vector>
#include <cmath> // for fabs
#include <cassert>
#include <algorithm>
#include <memory>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

//using namespace geos::algorithm;

namespace geos {
namespace geom { // geos::geom

/*protected*/
Polygon::Polygon(const Polygon& p)
    :
    Geometry(p),
    shell(detail::make_unique<LinearRing>(*p.shell)),
    holes(p.holes.size())
{
    for(std::size_t i = 0; i < holes.size(); ++i) {
        holes[i] = detail::make_unique<LinearRing>(*p.holes[i]);
    }
}

Polygon::Polygon(std::unique_ptr<LinearRing> && newShell,
                 const GeometryFactory& newFactory) :
        Geometry(&newFactory),
        shell(std::move(newShell))
{
    if(shell == nullptr) {
        shell = getFactory()->createLinearRing();
    }
}

Polygon::Polygon(std::unique_ptr<LinearRing> && newShell,
                 std::vector<std::unique_ptr<LinearRing>> && newHoles,
                 const GeometryFactory& newFactory) :
                 Geometry(&newFactory),
                 shell(std::move(newShell)),
                 holes(std::move(newHoles))
{
    if(shell == nullptr) {
        shell = getFactory()->createLinearRing();
    }

    // TODO move into validateConstruction() method
    if(shell->isEmpty() && hasNonEmptyElements(&holes)) {
        throw util::IllegalArgumentException("shell is empty but holes are not");
    }
    if (hasNullElements(&holes)) {
        throw util::IllegalArgumentException("holes must not contain null elements");
    }
}


std::unique_ptr<CoordinateSequence>
Polygon::getCoordinates() const
{
    if(isEmpty()) {
        return detail::make_unique<CoordinateSequence>(0u, hasZ(), hasM());
    }

    auto cl = detail::make_unique<CoordinateSequence>(0u, hasZ(), hasM());
    cl->reserve(getNumPoints());

    // Add shell points
    cl->add(*shell->getCoordinatesRO());

    // Add holes points
    for(const auto& hole : holes) {
        cl->add(*hole->getCoordinatesRO());
    }

    return cl;
}

size_t
Polygon::getNumPoints() const
{
    std::size_t numPoints = shell->getNumPoints();
    for(const auto& lr : holes) {
        numPoints += lr->getNumPoints();
    }
    return numPoints;
}

Dimension::DimensionType
Polygon::getDimension() const
{
    return Dimension::A; // area
}

uint8_t
Polygon::getCoordinateDimension() const
{
    uint8_t dimension = 2;

    if(shell != nullptr) {
        dimension = std::max(dimension, shell->getCoordinateDimension());
    }

    for(const auto& hole : holes) {
        dimension = std::max(dimension, hole->getCoordinateDimension());
    }

    return dimension;
}

bool
Polygon::hasM() const {
    if (shell->getCoordinatesRO()->hasM()) {
        return true;
    }

    for (const auto& hole : holes) {
        if (hole->getCoordinatesRO()->hasM()) {
            return true;
        }
    }

    return false;
}

bool
Polygon::hasZ() const {
    if (shell->getCoordinatesRO()->hasZ()) {
        return true;
    }

    for (const auto& hole : holes) {
        if (hole->getCoordinatesRO()->hasZ()) {
            return true;
        }
    }

    return false;
}

int
Polygon::getBoundaryDimension() const
{
    return 1;
}

bool
Polygon::isEmpty() const
{
    return shell->isEmpty();
}

const LinearRing*
Polygon::getExteriorRing() const
{
    return shell.get();
}

std::unique_ptr<LinearRing>
Polygon::releaseExteriorRing()
{
    return std::move(shell);
}

size_t
Polygon::getNumInteriorRing() const
{
    return holes.size();
}

const LinearRing*
Polygon::getInteriorRingN(std::size_t n) const
{
    return holes[n].get();
}

std::vector<std::unique_ptr<LinearRing>>
Polygon::releaseInteriorRings()
{
    return std::move(holes);
}

std::string
Polygon::getGeometryType() const
{
    return "Polygon";
}

// Returns a newly allocated Geometry object
/*public*/
std::unique_ptr<Geometry>
Polygon::getBoundary() const
{
    /*
     * We will make sure that what we
     * return is composed of LineString,
     * not LinearRings
     */

    const GeometryFactory* gf = getFactory();

    if(isEmpty()) {
        return std::unique_ptr<Geometry>(gf->createMultiLineString());
    }

    if(holes.empty()) {
        return std::unique_ptr<Geometry>(gf->createLineString(*shell));
    }

    std::vector<std::unique_ptr<Geometry>> rings(holes.size() + 1);

    rings[0] = gf->createLineString(*shell);
    for(std::size_t i = 0, n = holes.size(); i < n; ++i) {
        const LinearRing* hole = holes[i].get();
        std::unique_ptr<LineString> ls = gf->createLineString(*hole);
        rings[i + 1] = std::move(ls);
    }

    return getFactory()->createMultiLineString(std::move(rings));
}

bool
Polygon::equalsExact(const Geometry* other, double tolerance) const
{
    if(!isEquivalentClass(other)) {
        return false;
    }

    const Polygon* otherPolygon = detail::down_cast<const Polygon*>(other);
    if(! otherPolygon) {
        return false;
    }

    if(!shell->equalsExact(otherPolygon->shell.get(), tolerance)) {
        return false;
    }

    std::size_t nholes = holes.size();

    if(nholes != otherPolygon->holes.size()) {
        return false;
    }

    for(std::size_t i = 0; i < nholes; i++) {
        const LinearRing* hole = holes[i].get();
        const LinearRing* otherhole = otherPolygon->holes[i].get();
        if(!hole->equalsExact(otherhole, tolerance)) {
            return false;
        }
    }

    return true;
}

bool
Polygon::equalsIdentical(const Geometry* other_g) const
{
    if(!isEquivalentClass(other_g)) {
        return false;
    }

    const auto& other = static_cast<const Polygon&>(*other_g);

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

void
Polygon::apply_ro(CoordinateFilter* filter) const
{
    shell->apply_ro(filter);
    for(const auto& lr : holes) {
        lr->apply_ro(filter);
    }
}

void
Polygon::apply_rw(const CoordinateFilter* filter)
{
    shell->apply_rw(filter);
    for(auto& lr : holes) {
        lr->apply_rw(filter);
    }
}

void
Polygon::apply_rw(GeometryFilter* filter)
{
    filter->filter_rw(this);
}

void
Polygon::apply_ro(GeometryFilter* filter) const
{
    filter->filter_ro(this);
}

std::unique_ptr<Geometry>
Polygon::convexHull() const
{
    return getExteriorRing()->convexHull();
}


void
Polygon::normalize()
{
    normalize(shell.get(), true);
    for(auto& lr : holes) {
        normalize(lr.get(), false);
    }
    std::sort(holes.begin(), holes.end(), [](const std::unique_ptr<LinearRing> & a, const std::unique_ptr<LinearRing> & b) {
        return a->compareTo(b.get()) > 0;
    });
}

void
Polygon::orientRings(bool exteriorCW)
{
    shell->orient(exteriorCW);
    for (auto& hole : holes) {
        hole->orient(!exteriorCW);
    }
}

int
Polygon::compareToSameClass(const Geometry* g) const
{
    const Polygon* p = detail::down_cast<const Polygon*>(g);
    int shellComp = shell->compareToSameClass(p->shell.get());
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
        const LinearRing *lr = p->getInteriorRingN(i);
        const int holeComp = getInteriorRingN(i)->compareToSameClass(lr);
        if (holeComp != 0) {
            return holeComp;
        }
    }

    return 0;
}

/*
 * TODO: check this function, there should be CoordinateSequence copy
 *       reduction possibility.
 */
void
Polygon::normalize(LinearRing* ring, bool clockwise)
{
    if(ring->isEmpty()) {
        return;
    }

    const auto& ringCoords = ring->getCoordinatesRO();
    CoordinateSequence coords(0u, ringCoords->hasZ(), ringCoords->hasM());
    coords.reserve(ringCoords->size());

    // exclude last point (repeated)
    coords.add(*ringCoords, 0, ringCoords->size() - 2);

    const CoordinateXY* minCoordinate = coords.minCoordinate();

    CoordinateSequence::scroll(&coords, minCoordinate);
    coords.closeRing();

    if(algorithm::Orientation::isCCW(&coords) == clockwise) {
        coords.reverse();
    }
    ring->setPoints(&coords);
}

const CoordinateXY*
Polygon::getCoordinate() const
{
    return shell->getCoordinate();
}

/*
 *  Returns the area of this <code>Polygon</code>
 *
 * @return the area of the polygon
 */
double
Polygon::getArea() const
{
    double area = 0.0;
    area += algorithm::Area::ofRing(shell->getCoordinatesRO());
    for(const auto& lr : holes) {
        const CoordinateSequence* h = lr->getCoordinatesRO();
        area -= algorithm::Area::ofRing(h);
    }
    return area;
}

/**
 * Returns the perimeter of this <code>Polygon</code>
 *
 * @return the perimeter of the polygon
 */
double
Polygon::getLength() const
{
    double len = 0.0;
    len += shell->getLength();
    for(const auto& hole : holes) {
        len += hole->getLength();
    }
    return len;
}

void
Polygon::apply_ro(GeometryComponentFilter* filter) const
{
    filter->filter_ro(this);
    shell->apply_ro(filter);
    for(std::size_t i = 0, n = holes.size(); i < n && !filter->isDone(); ++i) {
        holes[i]->apply_ro(filter);
    }
}

void
Polygon::apply_rw(GeometryComponentFilter* filter)
{
    filter->filter_rw(this);
    shell->apply_rw(filter);
    for(std::size_t i = 0, n = holes.size(); i < n && !filter->isDone(); ++i) {
        holes[i]->apply_rw(filter);
    }
}

void
Polygon::apply_rw(CoordinateSequenceFilter& filter)
{
    shell->apply_rw(filter);

    if(! filter.isDone()) {
        for(std::size_t i = 0, n = holes.size(); i < n; ++i) {
            holes[i]->apply_rw(filter);
            if(filter.isDone()) {
                break;
            }
        }
    }
    if(filter.isGeometryChanged()) {
        geometryChanged();
    }
}

void
Polygon::apply_ro(CoordinateSequenceFilter& filter) const
{
    shell->apply_ro(filter);

    if(! filter.isDone()) {
        for(std::size_t i = 0, n = holes.size(); i < n; ++i) {
            holes[i]->apply_ro(filter);
            if(filter.isDone()) {
                break;
            }
        }
    }
    //if (filter.isGeometryChanged()) geometryChanged();
}

GeometryTypeId
Polygon::getGeometryTypeId() const
{
    return GEOS_POLYGON;
}

bool
Polygon::isRectangle() const
{
    if(getNumInteriorRing() != 0) {
        return false;
    }
    assert(shell != nullptr);
    if(shell->getNumPoints() != 5) {
        return false;
    }

    const CoordinateSequence& seq = *(shell->getCoordinatesRO());

    // check vertices have correct values
    const Envelope& env = *getEnvelopeInternal();
    for(uint32_t i = 0; i < 5; i++) {
        double x = seq.getX(i);
        if(!(x == env.getMinX() || x == env.getMaxX())) {
            return false;
        }
        double y = seq.getY(i);
        if(!(y == env.getMinY() || y == env.getMaxY())) {
            return false;
        }
    }

    // check vertices are in right order
    double prevX = seq.getX(0);
    double prevY = seq.getY(0);
    for(uint32_t i = 1; i <= 4; i++) {
        double x = seq.getX(i);
        double y = seq.getY(i);
        bool xChanged = (x != prevX);
        bool yChanged = (y != prevY);
        if(xChanged == yChanged) {
            return false;
        }
        prevX = x;
        prevY = y;
    }
    return true;
}

Polygon*
Polygon::reverseImpl() const
{
    if(isEmpty()) {
        return clone().release();
    }

    std::vector<std::unique_ptr<LinearRing>> interiorRingsReversed(holes.size());

    std::transform(holes.begin(),
                   holes.end(),
                   interiorRingsReversed.begin(),
    [](const std::unique_ptr<LinearRing> & g) {
        return g->reverse();
    });

    return getFactory()->createPolygon(shell->reverse(), std::move(interiorRingsReversed)).release();
}

} // namespace geos::geom
} // namespace geos
