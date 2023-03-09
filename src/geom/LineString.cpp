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
 * Last port: geom/LineString.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/util/IllegalArgumentException.h>
#include <geos/algorithm/Length.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h> // for getBoundary
#include <geos/geom/Envelope.h>
#include <geos/operation/BoundaryOp.h>
#include <geos/util.h>

#include <algorithm>
#include <typeinfo>
#include <memory>
#include <cassert>

using namespace geos::algorithm;

namespace geos {
namespace geom { // geos::geom

LineString::~LineString(){}

/*protected*/
LineString::LineString(const LineString& ls)
    :
    Geometry(ls),
    points(ls.points->clone()),
    envelope(ls.envelope)
{
}

/*public*/
LineString::LineString(CoordinateSequence::Ptr && newCoords,
                       const GeometryFactory& factory)
    :
    Geometry(&factory),
    points(newCoords ? std::move(newCoords) : detail::make_unique<CoordinateSequence>()),
    envelope(computeEnvelopeInternal())
{
    validateConstruction();
}

LineString*
LineString::reverseImpl() const
{
    if(isEmpty()) {
        return clone().release();
    }

    assert(points.get());
    auto seq = points->clone();
    seq->reverse();
    assert(getFactory());
    return getFactory()->createLineString(std::move(seq)).release();
}


/*private*/
void
LineString::validateConstruction()
{
    if(points.get() == nullptr) {
        points = detail::make_unique<CoordinateSequence>();
        return;
    }

    if(points->size() == 1) {
        throw util::IllegalArgumentException("point array must contain 0 or >1 elements\n");
    }
}

std::unique_ptr<CoordinateSequence>
LineString::getCoordinates() const
{
    assert(points.get());
    return points->clone();
    //return points;
}

const CoordinateSequence*
LineString::getCoordinatesRO() const
{
    assert(nullptr != points.get());
    return points.get();
}

std::unique_ptr<CoordinateSequence>
LineString::releaseCoordinates()
{
    auto newPts = detail::make_unique<CoordinateSequence>(0u, points->hasZ(), points->hasM());
    auto ret = std::move(points);
    points = std::move(newPts);
    geometryChanged();
    return ret;
}

const Coordinate&
LineString::getCoordinateN(std::size_t n) const
{
    assert(points.get());
    return points->getAt(n);
}

Dimension::DimensionType
LineString::getDimension() const
{
    return Dimension::L; // line
}

uint8_t
LineString::getCoordinateDimension() const
{
    return (uint8_t) points->getDimension();
}

bool
LineString::hasM() const
{
    return points->hasM();
}

bool
LineString::hasZ() const
{
    return points->hasZ();
}

int
LineString::getBoundaryDimension() const
{
    if(isClosed()) {
        return Dimension::False;
    }
    return 0;
}

bool
LineString::isEmpty() const
{
    assert(points.get());
    return points->isEmpty();
}

std::size_t
LineString::getNumPoints() const
{
    assert(points.get());
    return points->getSize();
}

std::unique_ptr<Point>
LineString::getPointN(std::size_t n) const
{
    assert(getFactory());
    assert(points.get());
    return std::unique_ptr<Point>(getFactory()->createPoint(points->getAt(n)));
}

std::unique_ptr<Point>
LineString::getStartPoint() const
{
    if(isEmpty()) {
        return nullptr;
    }
    return getPointN(0);
}

std::unique_ptr<Point>
LineString::getEndPoint() const
{
    if(isEmpty()) {
        return nullptr;
    }
    return getPointN(getNumPoints() - 1);
}

bool
LineString::isClosed() const
{
    if(isEmpty()) {
        return false;
    }

    return points->front<CoordinateXY>().equals2D(points->back<CoordinateXY>());
}

bool
LineString::isRing() const
{
    return isClosed() && isSimple();
}

std::string
LineString::getGeometryType() const
{
    return "LineString";
}

std::unique_ptr<Geometry>
LineString::getBoundary() const
{
    operation::BoundaryOp bop(*this);
    return bop.getBoundary();
}

bool
LineString::isCoordinate(Coordinate& pt) const
{
    assert(points.get());
    std::size_t npts = points->getSize();
    for(std::size_t i = 0; i < npts; i++) {
        if(points->getAt<CoordinateXY>(i) == pt) {
            return true;
        }
    }
    return false;
}

/*protected*/
Envelope
LineString::computeEnvelopeInternal() const
{
    if(isEmpty()) {
        return Envelope();
    }

    return points->getEnvelope();
}

bool
LineString::equalsExact(const Geometry* other, double tolerance) const
{
    if(!isEquivalentClass(other)) {
        return false;
    }

    const LineString* otherLineString = detail::down_cast<const LineString*>(other);
    std::size_t npts = points->getSize();
    if(npts != otherLineString->points->getSize()) {
        return false;
    }
    for(std::size_t i = 0; i < npts; ++i) {
        if(!equal(points->getAt<CoordinateXY>(i), otherLineString->points->getAt<CoordinateXY>(i), tolerance)) {
            return false;
        }
    }
    return true;
}

bool
LineString::equalsIdentical(const Geometry* other_g) const
{
    if(!isEquivalentClass(other_g)) {
        return false;
    }

    const auto& other = static_cast<const LineString&>(*other_g);

    if (envelope != other.envelope) {
        return false;
    }

    return getCoordinatesRO()->equalsIdentical(*other.getCoordinatesRO());
}

void
LineString::apply_rw(const CoordinateFilter* filter)
{
    assert(points.get());
    points->apply_rw(filter);
}

void
LineString::apply_ro(CoordinateFilter* filter) const
{
    assert(points.get());
    points->apply_ro(filter);
}

void
LineString::apply_rw(GeometryFilter* filter)
{
    assert(filter);
    filter->filter_rw(this);
}

void
LineString::apply_ro(GeometryFilter* filter) const
{
    assert(filter);
    filter->filter_ro(this);
}

/*private*/
void
LineString::normalizeClosed()
{
    if(isEmpty()) {
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

    if(coords->size() >= 4 && algorithm::Orientation::isCCW(coords.get())) {
        coords->reverse();
    }

    points = std::move(coords);
}

/*public*/
void
LineString::normalize()
{
    if (isEmpty()) return;
    assert(points.get());
    if (isClosed()) {
        normalizeClosed();
        return;
    }
    std::size_t npts = points->getSize();
    std::size_t n = npts / 2;
    for(std::size_t i = 0; i < n; i++) {
        std::size_t j = npts - 1 - i;
        if(!(points->getAt<CoordinateXY>(i) == points->getAt<CoordinateXY>(j))) {
            if(points->getAt<CoordinateXY>(i).compareTo(points->getAt<CoordinateXY>(j)) > 0) {
                points->reverse();
            }
            return;
        }
    }
}

int
LineString::compareToSameClass(const Geometry* ls) const
{
    const LineString* line = detail::down_cast<const LineString*>(ls);

    // MD - optimized implementation
    std::size_t mynpts = points->getSize();
    std::size_t othnpts = line->points->getSize();
    if(mynpts > othnpts) {
        return 1;
    }
    if(mynpts < othnpts) {
        return -1;
    }
    for(std::size_t i = 0; i < mynpts; i++) {
        int cmp = points->getAt<CoordinateXY>(i).compareTo(line->points->getAt<CoordinateXY>(i));
        if(cmp) {
            return cmp;
        }
    }
    return 0;
}

const CoordinateXY*
LineString::getCoordinate() const
{
    if(isEmpty()) {
        return nullptr;
    }
    return &(points->getAt<CoordinateXY>(0));
}

double
LineString::getLength() const
{
    return Length::ofLine(points.get());
}

void
LineString::apply_rw(GeometryComponentFilter* filter)
{
    assert(filter);
    filter->filter_rw(this);
}

void
LineString::apply_ro(GeometryComponentFilter* filter) const
{
    assert(filter);
    filter->filter_ro(this);
}

void
LineString::apply_rw(CoordinateSequenceFilter& filter)
{
    std::size_t npts = points->size();
    if(!npts) {
        return;
    }
    for(std::size_t i = 0; i < npts; ++i) {
        filter.filter_rw(*points, i);
        if(filter.isDone()) {
            break;
        }
    }
    if(filter.isGeometryChanged()) {
        geometryChanged();
    }
}

void
LineString::apply_ro(CoordinateSequenceFilter& filter) const
{
    std::size_t npts = points->size();
    if(!npts) {
        return;
    }
    for(std::size_t i = 0; i < npts; ++i) {
        filter.filter_ro(*points, i);
        if(filter.isDone()) {
            break;
        }
    }
    //if (filter.isGeometryChanged()) geometryChanged();
}

GeometryTypeId
LineString::getGeometryTypeId() const
{
    return GEOS_LINESTRING;
}

} // namespace geos::geom
} // namespace geos
