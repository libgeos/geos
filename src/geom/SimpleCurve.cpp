/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 2006 Refractions Research Inc.
 * Copyright (C) 2011 Sandro Santilli <strk@kbt.io>
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/SimpleCurve.h>

#include <geos/algorithm/CircularArcs.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/operation/BoundaryOp.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/util.h>


namespace geos {
namespace geom {

SimpleCurve::SimpleCurve(const SimpleCurve& other)
    : Curve(other),
      points(other.points->clone()),
      envelope(other.envelope)
{
}

SimpleCurve::SimpleCurve(std::unique_ptr<CoordinateSequence>&& newCoords,
                         bool isLinear,
                         const GeometryFactory& factory)
    : Curve(factory),
      points(newCoords ? std::move(newCoords) : std::make_unique<CoordinateSequence>()),
      envelope(computeEnvelopeInternal(isLinear))
{
}

void
SimpleCurve::apply_ro(CoordinateFilter* filter) const
{
    assert(points.get());
    points->apply_ro(filter);
}

void
SimpleCurve::apply_ro(CoordinateSequenceFilter& filter) const
{
    std::size_t npts = points->size();
    if (!npts) {
        return;
    }
    for (std::size_t i = 0; i < npts; ++i) {
        filter.filter_ro(*points, i);
        if (filter.isDone()) {
            break;
        }
    }
}

void
SimpleCurve::apply_rw(const CoordinateFilter* filter)
{
    assert(points.get());
    points->apply_rw(filter);
}

void
SimpleCurve::apply_rw(CoordinateSequenceFilter& filter)
{
    std::size_t npts = points->size();
    if (!npts) {
        return;
    }
    for (std::size_t i = 0; i < npts; ++i) {
        filter.filter_rw(*points, i);
        if (filter.isDone()) {
            break;
        }
    }
    if (filter.isGeometryChanged()) {
        geometryChanged();
    }
}

int
SimpleCurve::compareToSameClass(const Geometry* ls) const
{
    const SimpleCurve* line = detail::down_cast<const SimpleCurve*>(ls);

    // MD - optimized implementation
    std::size_t mynpts = points->getSize();
    std::size_t othnpts = line->points->getSize();
    if (mynpts > othnpts) {
        return 1;
    }
    if (mynpts < othnpts) {
        return -1;
    }
    for (std::size_t i = 0; i < mynpts; i++) {
        int cmp = points->getAt<CoordinateXY>(i).compareTo(line->points->getAt<CoordinateXY>(i));
        if (cmp) {
            return cmp;
        }
    }
    return 0;
}

Envelope
SimpleCurve::computeEnvelopeInternal(bool isLinear) const
{
    if (isEmpty()) {
        return Envelope();
    }

    if (isLinear) {
        return points->getEnvelope();
    }
    else {
        Envelope e;
        for (std::size_t i = 2; i < points->size(); i++) {
            algorithm::CircularArcs::expandEnvelope(e,
                                                    points->getAt<CoordinateXY>(i-2),
                                                    points->getAt<CoordinateXY>(i-1),
                                                    points->getAt<CoordinateXY>(i));
        }
        return e;
    }
}

bool
SimpleCurve::equalsExact(const Geometry* other, double tolerance) const
{
    if (!isEquivalentClass(other)) {
        return false;
    }

    const SimpleCurve* otherCurve = detail::down_cast<const SimpleCurve*>(other);
    std::size_t npts = points->getSize();
    if (npts != otherCurve->points->getSize()) {
        return false;
    }
    for (std::size_t i = 0; i < npts; ++i) {
        if (!equal(points->getAt<CoordinateXY>(i), otherCurve->points->getAt<CoordinateXY>(i), tolerance)) {
            return false;
        }
    }
    return true;
}

bool
SimpleCurve::equalsIdentical(const Geometry* other_g) const
{
    if (!isEquivalentClass(other_g)) {
        return false;
    }

    const auto& other = static_cast<const SimpleCurve&>(*other_g);

    if (envelope != other.envelope) {
        return false;
    }

    return getCoordinatesRO()->equalsIdentical(*other.getCoordinatesRO());
}

std::unique_ptr<Geometry>
SimpleCurve::getBoundary() const
{
    operation::BoundaryOp bop(*this);
    return bop.getBoundary();
}

const CoordinateXY*
SimpleCurve::getCoordinate() const
{
    if (isEmpty()) {
        return nullptr;
    }
    return &(points->getAt<CoordinateXY>(0));
}

uint8_t
SimpleCurve::getCoordinateDimension() const
{
    return (uint8_t) points->getDimension();
}

const Coordinate&
SimpleCurve::getCoordinateN(std::size_t n) const
{
    assert(points.get());
    return points->getAt(n);
}

std::unique_ptr<CoordinateSequence>
SimpleCurve::getCoordinates() const
{
    assert(points.get());
    return points->clone();
}

const CoordinateSequence*
SimpleCurve::getCoordinatesRO() const
{
    assert(nullptr != points.get());
    return points.get();
}

const SimpleCurve*
SimpleCurve::getCurveN(std::size_t) const
{
    return this;
}

std::unique_ptr<Point>
SimpleCurve::getEndPoint() const
{
    if (isEmpty()) {
        return nullptr;
    }
    return getPointN(getNumPoints() - 1);
}

std::size_t
SimpleCurve::getNumCurves() const
{
    return 1;
}

std::size_t
SimpleCurve::getNumPoints() const
{
    assert(points.get());
    return points->getSize();
}

std::unique_ptr<Point>
SimpleCurve::getPointN(std::size_t n) const
{
    assert(getFactory());
    assert(points.get());
    if (hasM() || hasZ()) {
        CoordinateXYZM c;
        points->getAt(n, c);
        return getFactory()->createPoint(c);
    }
    else {
        CoordinateXY c;
        points->getAt(n, c);
        return getFactory()->createPoint(c);
    }
}

std::unique_ptr<Point>
SimpleCurve::getStartPoint() const
{
    if (isEmpty()) {
        return nullptr;
    }
    return getPointN(0);
}

bool
SimpleCurve::hasM() const
{
    return points->hasM();
}

bool
SimpleCurve::hasZ() const
{
    return points->hasZ();
}

bool
SimpleCurve::isClosed() const
{
    if (isEmpty()) {
        return false;
    }

    return points->front<CoordinateXY>().equals2D(points->back<CoordinateXY>());
}

bool
SimpleCurve::isCoordinate(CoordinateXY& pt) const
{
    assert(points.get());
    std::size_t npts = points->getSize();
    for (std::size_t i = 0; i < npts; i++) {
        if (points->getAt<CoordinateXY>(i) == pt) {
            return true;
        }
    }
    return false;
}

bool
SimpleCurve::isEmpty() const
{
    assert(points.get());
    return points->isEmpty();
}

/*public*/
void
SimpleCurve::normalize()
{
    util::ensureNoCurvedComponents(*this);

    if (isEmpty()) return;
    assert(points.get());
    if (isClosed()) {
        normalizeClosed();
        return;
    }
    std::size_t npts = points->getSize();
    std::size_t n = npts / 2;
    for (std::size_t i = 0; i < n; i++) {
        std::size_t j = npts - 1 - i;
        if (!(points->getAt<CoordinateXY>(i) == points->getAt<CoordinateXY>(j))) {
            if (points->getAt<CoordinateXY>(i).compareTo(points->getAt<CoordinateXY>(j)) > 0) {
                points->reverse();
            }
            return;
        }
    }
}

/*private*/
void
SimpleCurve::normalizeClosed()
{
    if (isEmpty()) {
        return;
    }

    const auto& ringCoords = getCoordinatesRO();

    auto coords = detail::make_unique<CoordinateSequence>(0u, ringCoords->hasZ(), ringCoords->hasM());
    coords->reserve(ringCoords->size());
    // exclude last point (repeated)
    coords->add(*ringCoords, 0, ringCoords->size() - 2);

    const CoordinateXY* minCoordinate = coords->minCoordinate();

    CoordinateSequence::scroll(coords.get(), minCoordinate);
    coords->closeRing(true);

    if (coords->size() >= 4 && algorithm::Orientation::isCCW(coords.get())) {
        coords->reverse();
    }

    points = std::move(coords);
}

std::unique_ptr<CoordinateSequence>
SimpleCurve::releaseCoordinates()
{
    auto newPts = std::make_unique<CoordinateSequence>(0u, points->hasZ(), points->hasM());
    auto ret = std::move(points);
    points = std::move(newPts);
    geometryChanged();
    return ret;
}

} // namespace geos::geom
} // namespace geos
