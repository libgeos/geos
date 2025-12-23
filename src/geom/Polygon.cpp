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
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h> // for getBoundary()
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
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

std::unique_ptr<CoordinateSequence>
Polygon::getCoordinates() const
{
    if(isEmpty()) {
        return std::make_unique<CoordinateSequence>(0u, hasZ(), hasM());
    }

    auto cl = std::make_unique<CoordinateSequence>(0u, hasZ(), hasM());
    cl->reserve(getNumPoints());

    // Add shell points
    cl->add(*shell->getCoordinatesRO());

    // Add holes points
    for(const auto& hole : holes) {
        cl->add(*hole->getCoordinatesRO());
    }

    return cl;
}

Surface*
Polygon::getCurvedImpl(double distanceTolerance) const
{
    auto curvedShell = shell->getCurved(distanceTolerance);
    bool isCurved = curvedShell->hasCurvedComponents();

    if (holes.empty()) {
        if (isCurved) {
            return getFactory()->createCurvePolygon(std::move(curvedShell)).release();
        } else {
            return cloneImpl();
        }
    }

    std::vector<std::unique_ptr<Curve>> holesCurved(holes.size());
    for (std::size_t i = 0; i < holes.size(); i++) {
        holesCurved[i] = holes[i]->getCurved(distanceTolerance);
        isCurved |= holesCurved[i]->hasCurvedComponents();
    }

    if (isCurved) {
        return getFactory()->createCurvePolygon(std::move(curvedShell), std::move(holesCurved)).release();
    }

    return cloneImpl();
}

std::unique_ptr<Polygon>
Polygon::getLinearized(double degreeSpacing) const
{
    return std::unique_ptr<Polygon>(getLinearizedImpl(degreeSpacing));
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
        const auto& h = lr->getCoordinatesRO();
        area -= algorithm::Area::ofRing(h);
    }
    return area;
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

void
Polygon::orientRings(bool exteriorCW)
{
    shell->orient(exteriorCW);
    for (auto& hole : holes) {
        hole->orient(!exteriorCW);
    }
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

void
Polygon::normalize()
{
    normalize(shell.get(), true);
    for(auto& lr : holes) {
        normalize(lr.get(), false);
    }
    std::sort(holes.begin(), holes.end(), [](const auto& a, const auto& b) {
        return a->compareTo(b.get()) > 0;
    });
}

} // namespace geos::geom
} // namespace geos
