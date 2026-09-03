/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2016 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/distance/IndexedFacetDistance.java (f6187ee2 JTS-1.14)
 *
 **********************************************************************/

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/index/strtree/STRtree.h>
#include <geos/operation/distance/IndexedFacetDistance.h>
#include <geos/operation/distance/FacetSequence.h>
#include <geos/util/GEOSException.h>

using namespace geos::geom;
using namespace geos::index::strtree;

namespace geos {
namespace operation {
namespace distance {

/* public static */
double
IndexedFacetDistance::distance(const Geometry* g1, const Geometry* g2)
{
    IndexedFacetDistance ifd(g1);
    return ifd.distance(g2);
}

/* public static */
bool
IndexedFacetDistance::isWithinDistance(const Geometry* g1, const Geometry* g2, double distance)
{
    IndexedFacetDistance ifd(g1);
    return ifd.isWithinDistance(g2, distance);
}

/* public static */
std::unique_ptr<CoordinateSequence>
IndexedFacetDistance::nearestPoints(const Geometry* g1, const Geometry* g2)
{
    IndexedFacetDistance dist(g1);
    return dist.nearestPoints(g2);
}

double
IndexedFacetDistance::distance(const Geometry* g) const
{
    auto tree2 = FacetSequenceTreeBuilder::build(g);
    auto objs = cachedTree->nearestNeighbour<FacetDistance>(*tree2);
    if (!objs.first || !objs.second) {
        throw util::GEOSException("Cannot calculate IndexedFacetDistance on empty geometries.");
    }
    auto fs1 = static_cast<const FacetSequence*>(objs.first);
    auto fs2 = static_cast<const FacetSequence*>(objs.second);
    return fs1->distance(*fs2);
}

std::unique_ptr<geom::CoordinateSequence>
IndexedFacetDistance::nearestPoints(const geom::Geometry* g) const
{
    auto tree2 = FacetSequenceTreeBuilder::build(g);
    auto objs = cachedTree->nearestNeighbour<FacetDistance>(*tree2);
    if (!objs.first || !objs.second) {
        throw util::GEOSException("Cannot calculate IndexedFacetDistance on empty geometries.");
    }
    auto fs1 = static_cast<const FacetSequence*>(objs.first);
    auto fs2 = static_cast<const FacetSequence*>(objs.second);
    auto nearestPts = fs1->nearestLocations(*fs2);
    std::unique_ptr<CoordinateSequence> cs(new CoordinateSequence());
    cs->setPoints(nearestPts);
    return cs;
}


bool
IndexedFacetDistance::isWithinDistance(const Geometry* g, double maxDistance) const
{
    if (maxDistance < 0.0) maxDistance = 0.0;
    // short-circuit check
    double envDist = baseGeometry.getEnvelopeInternal()->distance(*g->getEnvelopeInternal());
    if (envDist > maxDistance) {
        return false;
    }

    //-- heuristic: for atomic indexed geom, test distance to envelope of test geom
    if (baseGeometry.getNumGeometries() == 1
        && ! g->getEnvelopeInternal()->contains(baseGeometry.getEnvelopeInternal()))
    {
        auto env2 = g->getEnvelope();
        if (distance(env2.get()) > maxDistance) {
            return false;
        }
    }

    auto tree2 = FacetSequenceTreeBuilder::build(g);
    return cachedTree->isWithinDistance<FacetDistance>(*tree2, maxDistance);
}

FacetNearestLocation
IndexedFacetDistance::nearestLocation(const geom::CoordinateXY& p) const
{
    CoordinateSequence seq{CoordinateXY(p.x, p.y)};
    FacetSequence query(&seq, 0, 1);
    FacetDistance itemDist;
    const FacetSequence* nearest = cachedTree->nearestNeighbour<FacetDistance>(
        *query.getEnvelope(), &query, itemDist);
    if (!nearest) {
        throw util::GEOSException("Cannot calculate IndexedFacetDistance on empty geometries.");
    }
    return nearest->nearestLocation(p);
}

geom::Coordinate
IndexedFacetDistance::nearestPoint(const geom::CoordinateXY& p) const
{
    return nearestLocation(p).pt;
}

double
IndexedFacetDistance::distance(const geom::CoordinateXY& p) const
{
    return p.distance(nearestPoint(p));
}

double
IndexedFacetDistance::distance(const geom::CoordinateXY& p0, const geom::CoordinateXY& p1) const
{
    CoordinateSequence seq{CoordinateXY(p0.x, p0.y), CoordinateXY(p1.x, p1.y)};
    FacetSequence query(&seq, 0, 2);
    FacetDistance itemDist;
    const FacetSequence* nearest = cachedTree->nearestNeighbour<FacetDistance>(
        *query.getEnvelope(), &query, itemDist);
    if (!nearest) {
        throw util::GEOSException("Cannot calculate IndexedFacetDistance on empty geometries.");
    }
    auto locs = nearest->nearestLocations(query);
    return locs[0].distance(locs[1]);
}


}
}
}
