/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2011 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/PointLocator.java 95fbe34b (JTS-1.15.2-SNAPSHOT)
 *
 **********************************************************************/

#include <geos/algorithm/PointLocator.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Location.h>
#include <geos/geomgraph/GeometryGraph.h>
#include <geos/util/UnsupportedOperationException.h>

#include <cassert>

using namespace geos::geom;

namespace geos {
namespace algorithm { // geos.algorithm


Location
PointLocator::locate(const CoordinateXY& p, const Geometry* geom)
{
    if (geom->isEmpty()) {
        return Location::EXTERIOR;
    }

    GeometryTypeId geomTypeId = geom->getGeometryTypeId();

    if (geomTypeId == GEOS_LINESTRING) {
        const LineString* ls_geom = static_cast<const LineString*>(geom);
        return locate(p, ls_geom);
    }

    if (geomTypeId == GEOS_POLYGON || geomTypeId == GEOS_CURVEPOLYGON) {
        const Surface* poly_geom = static_cast<const Surface*>(geom);
        return locate(p, poly_geom);
    }

    isIn = false;
    numBoundaries = 0;
    computeLocation(p, geom);
    if (geomgraph::GeometryGraph::isInBoundary(numBoundaries)) {
        return Location::BOUNDARY;
    }
    if (numBoundaries > 0 || isIn) {
        return Location::INTERIOR;
    }
    return Location::EXTERIOR;
}

/* private */
void
PointLocator::computeLocation(const CoordinateXY& p, const Geometry* geom)
{
    //-- handle empty elements
    if (geom->isEmpty()) {
        return;
    }
    
    GeometryTypeId geomTypeId = geom->getGeometryTypeId();
    switch (geomTypeId)
    {
        case GEOS_POINT: {
            const Point* pt = static_cast<const Point*>(geom);
            updateLocationInfo(locate(p, pt));
            break;
        }
        case GEOS_LINESTRING: {
            const LineString* ls = static_cast<const LineString*>(geom);
            updateLocationInfo(locate(p, ls));
            break;
        }
        case GEOS_LINEARRING: {
            const LineString* ls = static_cast<const LineString*>(geom);
            updateLocationInfo(locate(p, ls));
            break;
        }
        case GEOS_CIRCULARSTRING: {
            const CircularString* cs = static_cast<const CircularString*>(geom);
            updateLocationInfo(locate(p, cs));
            break;
        }
        case GEOS_COMPOUNDCURVE: {
            const CompoundCurve* cc = static_cast<const CompoundCurve*>(geom);
            updateLocationInfo(locate(p, cc));
            break;
        }
        case GEOS_POLYGON: {
            const Polygon* po = static_cast<const Polygon*>(geom);
            updateLocationInfo(locate(p, po));
            break;
        }
        case GEOS_MULTIPOINT: {
            const GeometryCollection* col = static_cast<const GeometryCollection*>(geom);
            for(const auto & g2 : *col) {
                computeLocation(p, g2.get());
            }
            break;
        }
        case GEOS_MULTILINESTRING: {
            const MultiLineString* mls = static_cast<const MultiLineString*>(geom);
            for(std::size_t i = 0, sz = mls->getNumGeometries(); i < sz; ++i) {
                const LineString* l = mls->getGeometryN(i);
                updateLocationInfo(locate(p, l));
            }
            break;
        }
        case GEOS_MULTIPOLYGON: {
            const MultiPolygon* mpo = static_cast<const MultiPolygon*>(geom);
            for(std::size_t i = 0, sz = mpo->getNumGeometries(); i < sz; ++i) {
                const Polygon* p_po = mpo->getGeometryN(i);
                updateLocationInfo(locate(p, p_po));
            }
            break;
        }
        case GEOS_GEOMETRYCOLLECTION: {
            const GeometryCollection* col = static_cast<const GeometryCollection*>(geom);
            for(const auto & g2 : *col) {
                computeLocation(p, g2.get());
            }
            break;
        }
        default: {
            throw util::UnsupportedOperationException("unknown GeometryTypeId");
        }
    }

}

/* private */
void
PointLocator::updateLocationInfo(geom::Location loc)
{
    if(loc == Location::INTERIOR) {
        isIn = true;
    }
    if(loc == Location::BOUNDARY) {
        ++numBoundaries;
    }
}

/* private */
Location
PointLocator::locate(const CoordinateXY& p, const Point* pt)
{
    // no point in doing envelope test, since equality test is just as fast
    const CoordinateXY* ptCoord = pt->getCoordinate();
    if(ptCoord != nullptr && ptCoord->equals2D(p)) {
        return Location::INTERIOR;
    }
    return Location::EXTERIOR;
}

/* private */
Location
PointLocator::locate(const CoordinateXY& p, const LineString* l)
{
    if(!l->getEnvelopeInternal()->intersects(p)) {
        return Location::EXTERIOR;
    }

    const CoordinateSequence* seq = l->getCoordinatesRO();
    if(! l->isClosed()) {
        if((p == seq->getAt<CoordinateXY>(0)) || (p == seq->getAt<CoordinateXY>(seq->getSize() - 1))) {
            return Location::BOUNDARY;
        }
    }
    if(PointLocation::isOnLine(p, seq)) {
        return Location::INTERIOR;
    }
    return Location::EXTERIOR;
}

/* private */
Location
PointLocator::locate(const CoordinateXY& p, const CircularString* cs)
{
    if(!cs->getEnvelopeInternal()->intersects(p)) {
        return Location::EXTERIOR;
    }

    if (cs->isClosed()) {
        const CoordinateSequence* seq = cs->getCoordinatesRO();
        if (p.equals2D(seq->front<CoordinateXY>())) {
            return Location::BOUNDARY;
        }
        if (p.equals2D(seq->back<CoordinateXY>())) {
            return Location::BOUNDARY;
        }
    }

    for (const auto& arc : cs->getArcs()) {
        if (arc.containsPoint(p)) {
            return Location::INTERIOR;
        }
    }

    return Location::EXTERIOR;
}

/* private */
Location
PointLocator::locate(const CoordinateXY& p, const CompoundCurve* cc)
{
    if(!cc->getEnvelopeInternal()->intersects(p)) {
        return Location::EXTERIOR;
    }

    if (cc->isClosed()) {
        const SimpleCurve* firstCurve = cc->getCurveN(0);
        if (p.equals2D(firstCurve->getCoordinatesRO()->front<CoordinateXY>())) {
            return Location::BOUNDARY;
        }

        const SimpleCurve* lastCurve = cc->getCurveN(cc->getNumCurves() - 1);
        if (p.equals2D(lastCurve->getCoordinatesRO()->back<CoordinateXY>())) {
            return Location::BOUNDARY;
        }
    }

    for (std::size_t i = 0; i < cc->getNumCurves(); i++) {
        const SimpleCurve* curve = cc->getCurveN(i);
        if (curve->getGeometryTypeId() == GEOS_CIRCULARSTRING) {
            const CircularString* cs = static_cast<const CircularString*>(curve);
            for (const auto& arc : cs->getArcs()) {
                if (arc.containsPoint(p)) {
                    return Location::INTERIOR;
                }
            }
        } else {
            const CoordinateSequence* seq = curve->getCoordinatesRO();
            if(PointLocation::isOnLine(p, seq)) {
                return Location::INTERIOR;
            }
        }
    }

    return Location::EXTERIOR;
}

/* private */
Location
PointLocator::locateInPolygonRing(const CoordinateXY& p, const Curve* ring)
{
    if(!ring->getEnvelopeInternal()->intersects(p)) {
        return Location::EXTERIOR;
    }

    return PointLocation::locateInRing(p, *ring);
}

/* private */
Location
PointLocator::locate(const CoordinateXY& p, const Surface* poly)
{
    if(poly->isEmpty()) {
        return Location::EXTERIOR;
    }

    const Curve* shell = poly->getExteriorRing();
    assert(shell);

    Location shellLoc = locateInPolygonRing(p, shell);
    if(shellLoc == Location::EXTERIOR) {
        return Location::EXTERIOR;
    }
    if(shellLoc == Location::BOUNDARY) {
        return Location::BOUNDARY;
    }

    // now test if the point lies in or on the holes
    for(std::size_t i = 0, n = poly->getNumInteriorRing(); i < n; ++i) {
        const Curve* hole = poly->getInteriorRingN(i);
        Location holeLoc = locateInPolygonRing(p, hole);
        if(holeLoc == Location::INTERIOR) {
            return Location::EXTERIOR;
        }
        if(holeLoc == Location::BOUNDARY) {
            return Location::BOUNDARY;
        }
    }
    return Location::INTERIOR;
}

} // namespace geos.algorithm
} // namespace geos

