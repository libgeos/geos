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
std::vector<geom::Coordinate>
IndexedFacetDistance::nearestPoints(const geom::Geometry* g1, const geom::Geometry* g2)
{
    IndexedFacetDistance dist(g1);
    return dist.nearestPoints(g2);
}

double
IndexedFacetDistance::distance(const Geometry* g) const
{
    struct FacetDistance {
        double operator()(const FacetSequence* a, const FacetSequence* b) {
            return a->distance(*b);
        }
    };

    auto tree2 = FacetSequenceTreeBuilder::build(g);
    auto nearest = cachedTree->nearestNeighbour<FacetDistance>(*tree2);

    if (!nearest.first) {
        throw util::GEOSException("Cannot calculate IndexedFacetDistance on empty geometries.");
    }

    return nearest.first->distance(*nearest.second);
}

std::vector<GeometryLocation>
IndexedFacetDistance::nearestLocations(const geom::Geometry* g) const
{
    struct FacetDistance {
        double operator()(const FacetSequence* a, const FacetSequence* b) const
        {
            return a->distance(*b);
        }
    };

    auto tree2 = FacetSequenceTreeBuilder::build(g);
    auto nearest = cachedTree->nearestNeighbour<FacetDistance>(*tree2);

    if (!nearest.first) {
        throw util::GEOSException("Cannot calculate IndexedFacetDistance on empty geometries.");
    }

    return nearest.first->nearestLocations(*nearest.second);
}

std::vector<Coordinate>
IndexedFacetDistance::nearestPoints(const geom::Geometry* g) const
{
    std::vector<GeometryLocation> minDistanceLocation = nearestLocations(g);
    std::vector<Coordinate> nearestPts;
    nearestPts.push_back(minDistanceLocation[0].getCoordinate());
    nearestPts.push_back(minDistanceLocation[1].getCoordinate());
    return nearestPts;
}

}
}
}
