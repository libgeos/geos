/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/snapround/SnapRoundingIntersectionAdder.h>
#include <geos/noding/snapround/HotPixel.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/NodingValidator.h>
#include <geos/noding/IntersectionFinderAdder.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Distance.h>
#include <geos/util.h>

#include <vector>
#include <exception>
#include <iostream>
#include <cassert>


using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround


/*public*/
void
SnapRoundingIntersectionAdder::processIntersections(
    SegmentString* e0, std::size_t segIndex0,
    SegmentString* e1, std::size_t segIndex1)
{
    // don't bother intersecting a segment with itself
    if (e0 == e1 && segIndex0 == segIndex1) return;

    const CoordinateSequence& seq0 = *e0->getCoordinates();
    const CoordinateSequence& seq1 = *e1->getCoordinates();

    li.computeIntersection(seq0, segIndex0, seq1, segIndex1);

    if (li.hasIntersection()) {
        if (li.isInteriorIntersection()) {
            for (std::size_t intIndex = 0, intNum = li.getIntersectionNum(); intIndex < intNum; intIndex++) {
                // Take a copy of the intersection coordinate
                intersections.add(li.getIntersection(intIndex));
            }
            static_cast<NodedSegmentString*>(e0)->addIntersections(&li, segIndex0, 0);
            static_cast<NodedSegmentString*>(e1)->addIntersections(&li, segIndex1, 1);
            return;
        }
    }

    /**
     * Segments did not actually intersect, within the limits of orientation index robustness.
     *
     * To avoid certain robustness issues in snap-rounding,
     * also treat very near vertex-segment situations as intersections.
     */
    processNearVertex(seq0, segIndex0, seq1, segIndex1, e1);
    processNearVertex(seq0, segIndex0 + 1, seq1, segIndex1, e1);
    processNearVertex(seq1, segIndex1, seq0, segIndex0, e0);
    processNearVertex(seq1, segIndex1 + 1, seq0, segIndex0, e0);
}

bool
SnapRoundingIntersectionAdder::isNearSegmentInterior(
    const geom::CoordinateXY& p, const geom::CoordinateXY& p0, const geom::CoordinateXY& p1) const
{
    if (p.distance(p0) < nearnessTol) return false;
    if (p.distance(p1) < nearnessTol) return false;

    double distSeg = algorithm::Distance::pointToSegment(p, p0, p1);
    return distSeg < nearnessTol;
}

/*private*/
void
SnapRoundingIntersectionAdder::processNearVertex(
        const CoordinateSequence& ptSeq, std::size_t ptIndex,
        const CoordinateSequence& segSeq, std::size_t segIndex,
        SegmentString* edge)
{
    const CoordinateXY& pt = ptSeq.getAt<CoordinateXY>(ptIndex);
    const CoordinateXY& seg0 = segSeq.getAt<CoordinateXY>(segIndex);
    const CoordinateXY& seg1 = segSeq.getAt<CoordinateXY>(segIndex + 1);
    if (isNearSegmentInterior(pt, seg0, seg1)) {
        intersections.add(ptSeq, ptIndex, ptIndex);
        static_cast<NodedSegmentString*>(edge)->addIntersection(intersections.back<CoordinateXYZM>(), segIndex);
    }
}

} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos
