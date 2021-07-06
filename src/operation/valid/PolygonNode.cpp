/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Quadrant.h>
#include <geos/operation/valid/PolygonNode.h>


namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using namespace geos::geom;


/* public static */
bool
PolygonNode::isCrossing(const Coordinate* nodePt, const Coordinate* a0, const Coordinate* a1, const Coordinate* b0, const Coordinate* b1)
{
    const Coordinate* aLo = a0;
    const Coordinate* aHi = a1;
    if (isAngleGreater(nodePt, aLo, aHi)) {
        aLo = a1;
        aHi = a0;
    }
    /**
     * Find positions of b0 and b1.
     * If they are the same they do not cross the other edge
     */
    bool isBetween0 = isBetween(nodePt, b0, aLo, aHi);
    bool isBetween1 = isBetween(nodePt, b1, aLo, aHi);

    return isBetween0 != isBetween1;
}


/* public static */
bool
PolygonNode::isInteriorSegment(const Coordinate* nodePt, const Coordinate* a0, const Coordinate* a1, const Coordinate* b)
{
    const Coordinate* aLo = a0;
    const Coordinate* aHi = a1;
    bool bIsInteriorBetween = true;
    if (isAngleGreater(nodePt, aLo, aHi)) {
        aLo = a1;
        aHi = a0;
        bIsInteriorBetween = false;
    }
    bool bIsBetween = isBetween(nodePt, b, aLo, aHi);
    bool bIsInterior = (bIsBetween && bIsInteriorBetween)
        || (! bIsBetween && ! bIsInteriorBetween);
    return bIsInterior;
}


/* private static */
bool
PolygonNode::isBetween(const Coordinate* origin, const Coordinate* p, const Coordinate* e0, const Coordinate* e1)
{
    bool isGreater0 = isAngleGreater(origin, p, e0);
    if (! isGreater0) return false;
    bool isGreater1 = isAngleGreater(origin, p, e1);
    return ! isGreater1;
}


/* private static */
bool
PolygonNode::isAngleGreater(const Coordinate* origin, const Coordinate* p, const Coordinate* q)
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
    int orient = algorithm::Orientation::index(*origin, *q, *p);
    return orient == algorithm::Orientation::COUNTERCLOCKWISE;
}


/* private static */
int
PolygonNode::quadrant(const Coordinate* origin, const Coordinate* p)
{
    double dx = p->x - origin->x;
    double dy = p->y - origin->y;
    return Quadrant::quadrant(dx, dy);
}


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
