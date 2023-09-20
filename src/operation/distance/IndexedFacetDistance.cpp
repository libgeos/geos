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
#include <geos/index/strtree/STRtree.h>
#include <geos/operation/distance/IndexedFacetDistance.h>

using namespace geos::geom;
using namespace geos::index::strtree;

namespace geos {
namespace operation {
namespace distance {

/*public static*/
double
IndexedFacetDistance::distance(const Geometry* g1, const Geometry* g2)
{
    IndexedFacetDistance ifd(g1);
    return ifd.distance(g2);
}

/*public static*/
std::unique_ptr<CoordinateSequence>
IndexedFacetDistance::nearestPoints(const geom::Geometry* g1, const geom::Geometry* g2)
{
    IndexedFacetDistance dist(g1);
    return dist.nearestPoints(g2);
}

double
IndexedFacetDistance::distance(const Geometry* g) const
{
    auto tree2 = FacetSequenceTreeBuilder::build(g);
    auto nearest = cachedTree->nearestNeighbour<FacetDistance>(*tree2);

    if (!nearest.first) {
        throw util::GEOSException("Cannot calculate IndexedFacetDistance on empty geometries.");
    }

    return nearest.first->distance(*nearest.second);
}

bool
IndexedFacetDistance::isWithinDistance(const Geometry* g, double maxDistance) const
{
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

std::vector<GeometryLocation>
IndexedFacetDistance::nearestLocations(const geom::Geometry* g) const
{

    auto tree2 = FacetSequenceTreeBuilder::build(g);
    auto nearest = cachedTree->nearestNeighbour<FacetDistance>(*tree2);

    if (!nearest.first) {
        throw util::GEOSException("Cannot calculate IndexedFacetDistance on empty geometries.");
    }

    return nearest.first->nearestLocations(*nearest.second);
}

std::unique_ptr<CoordinateSequence>
IndexedFacetDistance::nearestPoints(const geom::Geometry* g) const
{
    std::vector<GeometryLocation> minDistanceLocation = nearestLocations(g);
    auto nearestPts = detail::make_unique<CoordinateSequence>(2u);
    nearestPts->setAt(minDistanceLocation[0].getCoordinate(), 0);
    nearestPts->setAt(minDistanceLocation[1].getCoordinate(), 1);
    return nearestPts;
}

}
}
}
