/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/CircularArc.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util/UnsupportedOperationException.h>

namespace geos {
namespace geom {

/*public*/
CircularString::CircularString(std::unique_ptr<CoordinateSequence>&& newCoords,
                               const GeometryFactory& factory)
    :
    SimpleCurve(std::move(newCoords), false, factory)
{
    validateConstruction();
}

CircularString::CircularString(const std::shared_ptr<const CoordinateSequence>& newCoords,
                               const GeometryFactory& factory)
    :
    SimpleCurve(newCoords, false, factory)
{
    validateConstruction();
}

CircularString::~CircularString() = default;

std::unique_ptr<CircularString>
CircularString::clone() const
{
    return std::unique_ptr<CircularString>(cloneImpl());
}

void
CircularString::createArcs() const
{
    if (points->getSize() < 3) {
        return;
    }

    for (std::size_t i = 0; i < points->getSize() - 2; i += 2) {
        arcs.emplace_back(*points, i);
    }
}

const std::vector<CircularArc>&
CircularString::getArcs() const
{
    if (arcs.empty()) {
        createArcs();
    }
    return arcs;
}

std::string
CircularString::getGeometryType() const
{
    return "CircularString";
}

GeometryTypeId
CircularString::getGeometryTypeId() const
{
    return GEOS_CIRCULARSTRING;
}

double
CircularString::getLength() const
{
    if (isEmpty()) {
        return 0;
    }

    double tot = 0;
    for (const auto& arc : getArcs()) {
        tot += arc.getLength();
    }

    return tot;
}

void
CircularString::normalize()
{
    if (isEmpty()) return;

    assert(points.get());
    if (isClosed()) {
        normalizeClosed();
        return;
    }

    if (points->front<CoordinateXY>().compareTo( points->back<CoordinateXY>()) == 1) {
        if (points.use_count() > 1) {
            points = points->clone();
        }
        const_cast<CoordinateSequence*>(points.get())->reverse();
        arcs.clear();
    }
}

/*private*/
void
CircularString::normalizeClosed() {
    if (isEmpty()) {
        return;
    }

    const auto& ringCoords = getCoordinatesRO();
    const bool reverse = ringCoords->size() >= 4 && algorithm::Orientation::isCCW(ringCoords);

    std::size_t minInd = 0;
    const CoordinateXY* minPt = &ringCoords->getAt<CoordinateXY>(minInd);
    for (std::size_t i = 2; i < ringCoords->size() - 2; i++) {
        const CoordinateXY& pt = ringCoords->getAt<CoordinateXY>(i);
        if (pt.compareTo(*minPt) < 0) {
            minInd = i;
            minPt = &pt;
        }
    }

    if (minInd > 0) {
        auto coords = std::make_shared<CoordinateSequence>(0u, ringCoords->hasZ(), ringCoords->hasM());
        coords->reserve(ringCoords->size());

        coords->add(*ringCoords, minInd, ringCoords->size() - 2);
        coords->add(*ringCoords, 0, minInd);

        if (reverse) {
            coords->reverse();
        }
        points = std::move(coords);
    } else if (reverse) {
        if (points.use_count() > 1) {
            points = points->clone();
        }
        const_cast<CoordinateSequence*>(points.get())->reverse();
    }
}

std::unique_ptr<Curve>
CircularString::getCurved(const algorithm::LineToCurveParams&) const
{
    return getFactory()->createCircularString(points);
}


LineString*
CircularString::getLinearizedImpl(const algorithm::CurveToLineParams& params) const {
    if (isEmpty()) {
        return getFactory()->createLineString().release();
    }

    auto seq = std::make_shared<CoordinateSequence>(0, hasZ(), hasM());
    seq->add(*getCoordinatesRO(), static_cast<std::size_t>(0), 0);

    for (const CircularArc& arc : getArcs())
    {
        arc.addLinearizedPoints(*seq, params);
    }

    return getFactory()->createLineString(seq).release();
}

CircularString*
CircularString::reverseImpl() const
{
    if (isEmpty()) {
        return clone().release();
    }

    assert(points.get());
    auto seq = points->clone();
    seq->reverse();
    assert(getFactory());
    return getFactory()->createCircularString(std::move(seq)).release();
}

void
CircularString::validateConstruction()
{
    if (points.get() == nullptr) {
        points = std::make_unique<CoordinateSequence>();
        return;
    }

    if (points->size() > 0 && (points->size() < 3 || points->size() % 2 == 0)) {
        throw util::IllegalArgumentException("point array size must zero or be an odd number >= 3");
    }
}

}
}
