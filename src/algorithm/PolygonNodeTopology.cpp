/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://libgeos.org
 *
 * Copyright (c) 2021 Martin Davis
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/PolygonNodeTopology.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Quadrant.h>

using geos::geom::Coordinate;
using geos::geom::Quadrant;

namespace geos {
namespace algorithm { // geos.algorithm

/* public static */
bool
PolygonNodeTopology::isCrossing(const CoordinateXY* nodePt,
    const CoordinateXY* a0, const CoordinateXY* a1,
    const CoordinateXY* b0, const CoordinateXY* b1)
{
    const CoordinateXY* aLo = a0;
    const CoordinateXY* aHi = a1;
    if (isAngleGreater(nodePt, aLo, aHi)) {
        aLo = a1;
        aHi = a0;
    }
    /**
     * Find positions of b0 and b1.
     * If they are the same they do not cross the other edge
     */
    bool bBetween0 = isBetween(nodePt, b0, aLo, aHi);
    bool bBetween1 = isBetween(nodePt, b1, aLo, aHi);

    return bBetween0 != bBetween1;
}

/* public static */
bool
PolygonNodeTopology::isInteriorSegment(const CoordinateXY* nodePt,
    const CoordinateXY* a0, const CoordinateXY* a1, const CoordinateXY* b)
{
    const CoordinateXY* aLo = a0;
    const CoordinateXY* aHi = a1;
    bool isInteriorBetween = true;
    if (isAngleGreater(nodePt, aLo, aHi)) {
        aLo = a1;
        aHi = a0;
        isInteriorBetween = false;
    }
    bool bBetween = isBetween(nodePt, b, aLo, aHi);
    bool isInterior = (bBetween && isInteriorBetween)
        || (! bBetween && ! isInteriorBetween);
    return isInterior;
}

/* private static */
bool
PolygonNodeTopology::isBetween(const CoordinateXY* origin,
    const CoordinateXY* p,
    const CoordinateXY* e0, const CoordinateXY* e1)
{
    bool isGreater0 = isAngleGreater(origin, p, e0);
    if (! isGreater0) return false;
    bool isGreater1 = isAngleGreater(origin, p, e1);
    return ! isGreater1;
}


/* private static */
bool
PolygonNodeTopology::isAngleGreater(const CoordinateXY* origin,
    const CoordinateXY* p, const CoordinateXY* q)
{
    int quadrantP = quadrant(origin, p);
    int quadrantQ = quadrant(origin, q);

    /**
     * If the vectors are in different quadrants,
     * that determines the ordering
     */
    if (quadrantP > quadrantQ) return true;
    if (quadrantP < quadrantQ) return false;

    //--- vectors are in the same quadrant
    // Check relative orientation of vectors
    // P > Q if it is CCW of Q
    int orient = Orientation::index(*origin, *q, *p);
    return orient == Orientation::COUNTERCLOCKWISE;
}


/* private static */
int
PolygonNodeTopology::quadrant(const CoordinateXY* origin, const CoordinateXY* p)
{
    double dx = p->x - origin->x;
    double dy = p->y - origin->y;
    return Quadrant::quadrant(dx,  dy);
}



} // namespace geos.algorithm
} //namespace geos

