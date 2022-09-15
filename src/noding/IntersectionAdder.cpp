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
 * Last port: noding/IntersectionAdder.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#include <geos/noding/IntersectionAdder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/util.h>

using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

/*private*/
bool
IntersectionAdder::isTrivialIntersection(const algorithm::LineIntersector::IntersectionResult& result,
                                         const SegmentString* e0, std::size_t segIndex0,
                                         const SegmentString* e1, std::size_t segIndex1)
{
    if(e0 != e1) {
        return false;
    }

    if(result.getIntersectionNum() != 1) {
        return false;
    }

    if(isAdjacentSegments(segIndex0, segIndex1)) {
        return true;
    }

    if(! e0->isClosed()) {
        return false;
    }

    auto maxSegIndex = e0->size() - 1;
    if((segIndex0 == 0 && segIndex1 == maxSegIndex)
            || (segIndex1 == 0 && segIndex0 == maxSegIndex)) {
        return true;
    }
    return false;

}

/*public*/
void
IntersectionAdder::processIntersections(
    SegmentString* e0,  std::size_t segIndex0,
    SegmentString* e1,  std::size_t segIndex1)
{
    // don't bother intersecting a segment with itself
    if(e0 == e1 && segIndex0 == segIndex1) {
        return;
    }

    numTests++;


    const Coordinate& p00 = e0->getCoordinate(segIndex0);
    const Coordinate& p01 = e0->getCoordinate(segIndex0 + 1);
    const Coordinate& p10 = e1->getCoordinate(segIndex1);
    const Coordinate& p11 = e1->getCoordinate(segIndex1 + 1);

    const auto& result = li.computeIntersection(p00, p01, p10, p11);

    // No intersection, nothing to do
    if(!result.hasIntersection()) {
        return;
    }

    //intersectionFound = true;
    numIntersections++;

    if(result.isInterior()) {
        numInteriorIntersections++;
        hasInterior = true;
    }

    // if the segments are adjacent they have at least
    // one trivial intersection,
    // the shared endpoint.  Don't bother adding it if it
    // is the only intersection.
    if(! isTrivialIntersection(result, e0, segIndex0, e1, segIndex1)) {
        hasIntersectionVar = true;

        NodedSegmentString* ee0 = detail::down_cast<NodedSegmentString*>(e0);
        NodedSegmentString* ee1 = detail::down_cast<NodedSegmentString*>(e1);
        ee0->addIntersections(result, segIndex0, 0);
        ee1->addIntersections(result, segIndex1, 1);

        if(result.isProper()) {
            numProperIntersections++;
            //Debug.println(li.toString());
            //Debug.println(li.getIntersection(0));
            properIntersectionPoint = result.getIntersection(0);
            hasProper = true;
            hasProperInterior = true;
        }
    }
}

} // namespace geos.noding
} // namespace geos
