/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/NodingIntersectionFinder.java rev. ??? (JTS-1.8)
 *
 **********************************************************************/

#include <geos/noding/NodingIntersectionFinder.h>
#include <geos/noding/SegmentString.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>

#include <cstdlib>

using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding


/* public (override) */
void
NodingIntersectionFinder::processIntersections(
    SegmentString* e0,  std::size_t segIndex0,
    SegmentString* e1,  std::size_t segIndex1)
{
    using geos::geom::Coordinate;

    // short-circuit if intersection already found
    if(!findAllIntersections && hasIntersection()) {
        return;
    }

    // don't bother intersecting a segment with itself
    bool isSameSegString = e0 == e1;
    bool isSameSegment = isSameSegString && segIndex0 == segIndex1;
    if(isSameSegment) {
        return;
    }

    /*
     * If enabled, only test end segments (on either segString).
     */
    if(isCheckEndSegmentsOnly) {
        bool isEndSegPresent =
            isEndSegment(e0, segIndex0) ||
            isEndSegment(e1, segIndex1);
        if(! isEndSegPresent) {
            return;
        }
    }

    const Coordinate& p00 = e0->getCoordinate(segIndex0);
    const Coordinate& p01 = e0->getCoordinate(segIndex0 + 1);
    const Coordinate& p10 = e1->getCoordinate(segIndex1);
    const Coordinate& p11 = e1->getCoordinate(segIndex1 + 1);

    bool isEnd00 = segIndex0 == 0;
    bool isEnd01 = segIndex0 + 2 == e0->size();
    bool isEnd10 = segIndex1 == 0;
    bool isEnd11 = segIndex1 + 2 == e1->size();

    li.computeIntersection(p00, p01, p10, p11);
    /**
     * Check for an intersection in the interior of a segment
     */
    bool isInteriorInt = li.hasIntersection() && li.isInteriorIntersection();
    /**
     * Check for an intersection between two vertices which are not both endpoints.
     */
    std::size_t segDiff = std::max(segIndex0, segIndex1) - std::min(segIndex0, segIndex1);
    bool isAdjacentSegment = isSameSegString && segDiff <= 1;
    bool isInteriorVertexInt = (!isAdjacentSegment) &&
                               isInteriorVertexIntersection(p00, p01, p10, p11, isEnd00, isEnd01, isEnd10, isEnd11);

    if(isInteriorInt || isInteriorVertexInt) {
        // found an intersection!
        intSegments.push_back(p00);
        intSegments.push_back(p01);
        intSegments.push_back(p10);
        intSegments.push_back(p11);

        interiorIntersection = li.getIntersection(0);
        // TODO: record endpoint intersection(s)
        // if (keepIntersections) intersections.add(interiorIntersection);
        intersectionCount++;
    }
}

} // namespace geos.noding
} // namespace geos

