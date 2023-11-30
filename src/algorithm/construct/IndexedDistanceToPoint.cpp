/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Martin Davis <mtnclimb@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/construct/IndexedDistanceToPoint.java
 * https://github.com/locationtech/jts/commit/d92f783163d9440fcc10c729143787bf7b9fe8f9
 *
 **********************************************************************/

#include <geos/algorithm/construct/IndexedDistanceToPoint.h>
#include <geos/geom/Location.h>

//using namespace geos::geom;
using geos::geom::Location;

namespace geos {
namespace algorithm { // geos.algorithm
namespace construct { // geos.algorithm.construct

IndexedDistanceToPoint::IndexedDistanceToPoint(const Geometry& geom)
    : targetGeometry(geom)
{
}

/* private */
void IndexedDistanceToPoint::init()
{
    if (facetDistance != nullptr)
        return;
    ptLocator.reset(new IndexedPointInPolygonsLocator(targetGeometry));
    facetDistance.reset(new IndexedFacetDistance(&targetGeometry));
}

/* public */
double IndexedDistanceToPoint::distance(const Point& pt)
{
    init();
    //-- distance is 0 if point is inside a target polygon
    if (isInArea(pt)) {
        return 0;
    }
    return facetDistance->distance(&pt);
}

/* private */
bool IndexedDistanceToPoint::isInArea(const Point& pt)
{
    return Location::EXTERIOR != ptLocator->locate(pt.getCoordinate());
}

/* public */
std::unique_ptr<geom::CoordinateSequence> 
IndexedDistanceToPoint::nearestPoints(const Point& pt)
{
    init();
    return facetDistance->nearestPoints(&pt);
}

}}}
