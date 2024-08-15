/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/prep/BasicPreparedGeometry.java rev. 1.5 (JTS-1.10)
 *
 **********************************************************************/


#include <geos/geom/prep/BasicPreparedGeometry.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/geom/util/ComponentCoordinateExtracter.h>
#include <geos/operation/distance/DistanceOp.h>

#include "geos/util.h"

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep

/*            *
 * protected: *
 *            */

void
BasicPreparedGeometry::setGeometry(const geom::Geometry* geom)
{
    baseGeom = geom;
    geom::util::ComponentCoordinateExtracter::getCoordinates(*baseGeom, representativePts);
}

bool
BasicPreparedGeometry::envelopesIntersect(const geom::Geometry* g) const
{
    if (g->getGeometryTypeId() == GEOS_POINT) {
        auto pt = g->getCoordinate();
        if (pt == nullptr) {
            return false;
        }
        return baseGeom->getEnvelopeInternal()->intersects(*pt);
    }

    return baseGeom->getEnvelopeInternal()->intersects(g->getEnvelopeInternal());
}

bool
BasicPreparedGeometry::envelopeCovers(const geom::Geometry* g) const
{
    if (g->getGeometryTypeId() == GEOS_POINT) {
        auto pt = g->getCoordinate();
        if (pt == nullptr) {
            return false;
        }
        return baseGeom->getEnvelopeInternal()->covers(pt);
    }

    return baseGeom->getEnvelopeInternal()->covers(g->getEnvelopeInternal());
}

/*
 * public:
 */
BasicPreparedGeometry::BasicPreparedGeometry(const Geometry* geom)
{
    setGeometry(geom);
}

bool
BasicPreparedGeometry::isAnyTargetComponentInTest(const geom::Geometry* testGeom) const
{
    algorithm::PointLocator locator;

    for(const auto& c : representativePts) {
        if(locator.intersects(*c, testGeom)) {
            return true;
        }
    }
    return false;
}

bool
BasicPreparedGeometry::within(const geom::Geometry* g) const
{
    return getRelateNG().within(g);
}

bool
BasicPreparedGeometry::contains(const geom::Geometry* g) const
{
    return getRelateNG().contains(g);
}

bool
BasicPreparedGeometry::containsProperly(const geom::Geometry* g)	const
{
    return getRelateNG().relate(g, "T**FF*FF*");
}

bool
BasicPreparedGeometry::coveredBy(const geom::Geometry* g) const
{
    return getRelateNG().coveredBy(g);
}

bool
BasicPreparedGeometry::covers(const geom::Geometry* g) const
{
    return getRelateNG().covers(g);
}

bool
BasicPreparedGeometry::crosses(const geom::Geometry* g) const
{
    return getRelateNG().crosses(g);
}

bool
BasicPreparedGeometry::disjoint(const geom::Geometry* g)	const
{
    return getRelateNG().disjoint(g);
}

bool
BasicPreparedGeometry::intersects(const geom::Geometry* g) const
{
    return getRelateNG().intersects(g);
}

bool
BasicPreparedGeometry::overlaps(const geom::Geometry* g)	const
{
    return getRelateNG().overlaps(g);
}

bool
BasicPreparedGeometry::touches(const geom::Geometry* g) const
{
    return getRelateNG().touches(g);
}

bool
BasicPreparedGeometry::relate(const geom::Geometry* g, const std::string& pat) const
{
    return getRelateNG().relate(g, pat);
}

std::unique_ptr<IntersectionMatrix>
BasicPreparedGeometry::relate(const geom::Geometry* g) const
{
    return getRelateNG().relate(g);
}


std::unique_ptr<geom::CoordinateSequence>
BasicPreparedGeometry::nearestPoints(const geom::Geometry* g) const
{
    operation::distance::DistanceOp dist(baseGeom, g);
    return dist.nearestPoints();
}

double
BasicPreparedGeometry::distance(const geom::Geometry* g) const
{
    std::unique_ptr<geom::CoordinateSequence> coords = nearestPoints(g);
    if ( ! coords ) return DoubleInfinity;
    return coords->getAt(0).distance( coords->getAt(1) );
}

bool
BasicPreparedGeometry::isWithinDistance(const geom::Geometry* g, double dist) const
{
    return baseGeom->isWithinDistance(g, dist);
}

std::string
BasicPreparedGeometry::toString()
{
    return baseGeom->toString();
}

} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos
