/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/PointLocation.h>
#include <geos/algorithm/RayCrossingCounter.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Location.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm
namespace locate { // geos.algorithm

/**
 * locate is the main location function.  It handles both single-element
 * and multi-element Geometries.  The algorithm for multi-element Geometries
 * is more complex, since it has to take into account the boundaryDetermination rule
 */
geom::Location
SimplePointInAreaLocator::locate(const CoordinateXY& p, const Geometry* geom)
{
    return locateInGeometry(p, geom);
}

bool
SimplePointInAreaLocator::isContained(const CoordinateXY& p, const Geometry* geom)
{
    return Location::EXTERIOR != locate(p, geom);
}

bool
SimplePointInAreaLocator::isAnyPointContained(const geom::Geometry& pt, const geom::Geometry& area)
{
    // Empty point sets do not intersect any area.
    if (pt.isEmpty()) {
        return false;
    }
    if (pt.getNumGeometries() > 1) {
        for (size_t i = 0; i < pt.getNumGeometries(); i++ ) {
            if (isAnyPointContained(*pt.getGeometryN(i), area)) {
                return true;
            }
        }
        return false;
    }

    const CoordinateXY* c = pt.getCoordinate();
    if (c == nullptr) {
        return false;
    }
    return isContained(*c, &area);
}

bool
SimplePointInAreaLocator::isEveryPointContained(const geom::Geometry &pt, const geom::Geometry &area)
{
    // Vacuous truth: every point of an empty set is contained.
    if (pt.isEmpty()) {
        return true;
    }
    if (pt.getNumGeometries() > 1) {
        for (size_t i = 0; i < pt.getNumGeometries(); i++ ) {
            if (!isEveryPointContained(*pt.getGeometryN(i), area)) {
                return false;
            }
        }
        return true;
    }

    const CoordinateXY* c = pt.getCoordinate();
    if (c == nullptr) {
        return true;
    }
    return isContained(*c, &area);
}

bool
SimplePointInAreaLocator::isAreaContainingPoints(const geom::Geometry& pt, const geom::Geometry& area)
{
    // Contains requires II=T; empty point geometry cannot satisfy that.
    if (pt.isEmpty()) {
        return false;
    }

    bool anyInterior = false;
    const std::size_t n = pt.getNumGeometries();

    if (n > 1) {
        for (std::size_t i = 0; i < n; i++) {
            const geom::Geometry* gi = pt.getGeometryN(i);
            if (gi->isEmpty()) {
                continue;
            }
            if (gi->getNumGeometries() > 1) {
                if (!isAreaContainingPoints(*gi, area)) {
                    return false;
                }
                anyInterior = true;
                continue;
            }
            const CoordinateXY* c = gi->getCoordinate();
            if (c == nullptr) {
                continue;
            }
            const Location loc = locate(*c, &area);
            if (loc == Location::EXTERIOR) {
                return false;
            }
            if (loc == Location::INTERIOR) {
                anyInterior = true;
            }
        }
        return anyInterior;
    }

    const CoordinateXY* c = pt.getCoordinate();
    if (c == nullptr) {
        return false;
    }
    return locate(*c, &area) == Location::INTERIOR;
}

geom::Location
SimplePointInAreaLocator::locateInGeometry(const CoordinateXY& p, const Geometry* geom)
{
    /*
     * Do a fast check against the geometry envelope first
     */
    if (! geom->getEnvelopeInternal()->intersects(p))
        return Location::EXTERIOR;

    if (geom->getDimension() < 2) {
        return Location::EXTERIOR;
    }

    if (geom->getNumGeometries() == 1) {
        auto typ = geom->getGeometryTypeId();
        if (typ == GEOS_POLYGON || typ == GEOS_CURVEPOLYGON) {
            auto surface = static_cast<const Surface*>(geom);
            return locatePointInSurface(p, *surface);
        }
    }
    for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
        const Geometry* gi = geom->getGeometryN(i);
        auto loc = locateInGeometry(p, gi);
        if(loc != Location::EXTERIOR) {
            return loc;
        }
    }

    return Location::EXTERIOR;
}

geom::Location
SimplePointInAreaLocator::locatePointInSurface(const CoordinateXY& p, const Surface& surface)
{
    if(surface.isEmpty()) {
        return Location::EXTERIOR;
    }
    if(!surface.getEnvelopeInternal()->contains(p)) {
        return Location::EXTERIOR;
    }
    const Curve& shell = *surface.getExteriorRing();
    Location shellLoc = PointLocation::locateInRing(p, shell);
    if(shellLoc != Location::INTERIOR) {
        return shellLoc;
    }

    // now test if the point lies in or on the holes
    for(std::size_t i = 0; i < surface.getNumInteriorRing(); i++) {
        const Curve& hole = *surface.getInteriorRingN(i);
        if(hole.getEnvelopeInternal()->contains(p)) {
            Location holeLoc = RayCrossingCounter::locatePointInRing(p, hole);
            if(holeLoc == Location::BOUNDARY) {
                return Location::BOUNDARY;
            }
            if(holeLoc == Location::INTERIOR) {
                return Location::EXTERIOR;
            }
            // if in EXTERIOR of this hole, keep checking other holes
        }
    }
    return Location::INTERIOR;
}

} // namespace geos.algorithm.locate
} // namespace geos.algorithm
} // namespace geos
