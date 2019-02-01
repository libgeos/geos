/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/Orientation.java @ 2017-09-04
 *
 **********************************************************************/

#include <cmath>
#include <vector>

#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Location.h>
#include <geos/util/IllegalArgumentException.h>

namespace geos {
namespace algorithm { // geos.algorithm

/* public static */
int
Orientation::index(const geom::Coordinate& p1, const geom::Coordinate& p2,
                   const geom::Coordinate& q)
{
    return CGAlgorithmsDD::orientationIndex(p1, p2, q);
}

/* public static */
bool
Orientation::isCCW(const geom::CoordinateSequence* ring)
{
    // sanity check
    if(ring->getSize() < 4) {
        throw util::IllegalArgumentException("Ring has fewer than 4 points, so orientation cannot be determined");
    }

    // # of points without closing endpoint
    const std::size_t nPts = ring->getSize() - 1;

    // find highest point
    const geom::Coordinate* hiPt = &ring->getAt(0);
    size_t hiIndex = 0;
    for(std::size_t i = 1; i <= nPts; ++i) {
        const geom::Coordinate* p = &ring->getAt(i);
        if(p->y > hiPt->y) {
            hiPt = p;
            hiIndex = i;
        }
    }

    // find distinct point before highest point
    auto iPrev = hiIndex;
    do {
        if(iPrev == 0) {
            iPrev = nPts;
        }
        iPrev = iPrev - 1;
    }
    while(ring->getAt(iPrev) == *hiPt && iPrev != hiIndex);

    // find distinct point after highest point
    auto iNext = hiIndex;
    do {
        iNext = (iNext + 1) % nPts;
    }
    while(ring->getAt(iNext) == *hiPt && iNext != hiIndex);

    const geom::Coordinate* prev = &ring->getAt(iPrev);
    const geom::Coordinate* next = &ring->getAt(iNext);

    /*
     * This check catches cases where the ring contains an A-B-A
     * configuration of points.
     * This can happen if the ring does not contain 3 distinct points
     * (including the case where the input array has fewer than 4 elements),
     * or it contains coincident line segments.
     */
    if(prev->equals2D(*hiPt) || next->equals2D(*hiPt) ||
            prev->equals2D(*next)) {
        return false;
        // MD - don't bother throwing exception,
        // since this isn't a complete check for ring validity
        //throw  IllegalArgumentException("degenerate ring (does not contain 3 distinct points)");
    }

    int disc = Orientation::index(*prev, *hiPt, *next);

    /**
     *  If disc is exactly 0, lines are collinear.
     * There are two possible cases:
     *  (1) the lines lie along the x axis in opposite directions
     *  (2) the lines lie on top of one another
     *
     *  (1) is handled by checking if next is left of prev ==> CCW
     *  (2) should never happen, so we're going to ignore it!
     *  (Might want to assert this)
     */
    bool isCCW = false;

    if(disc == 0) {
        // poly is CCW if prev x is right of next x
        isCCW = (prev->x > next->x);
    }
    else {
        // if area is positive, points are ordered CCW
        isCCW = (disc > 0);
    }

    return isCCW;
}



} // namespace geos.algorithm
} //namespace geos

