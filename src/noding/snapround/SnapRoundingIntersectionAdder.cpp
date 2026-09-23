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
#include <geos/math/DD.h>
#include <geos/util.h>

#include <vector>
#include <exception>
#include <iostream>
#include <cassert>
#include <algorithm>
#include <cmath>


using namespace geos::algorithm;
using namespace geos::geom;

namespace {

double
coordinateInCell(double ordinate, double rounded, const geos::geom::PrecisionModel& pm)
{
    if (pm.makePrecise(ordinate) == rounded) {
        return ordinate;
    }
    // The exact intersection may straddle a grid boundary hidden by the
    // final double conversion. Move by at most one representable value so
    // the original intersection location changes as little as possible.
    double adjacent = std::nextafter(ordinate, rounded);
    return pm.makePrecise(adjacent) == rounded ? adjacent : ordinate;
}

double
roundIntersectionOrdinate(const geos::math::DD& ordinate,
                          const geos::geom::PrecisionModel& pm)
{
    using geos::math::DD;
    if (pm.getGridSize() > 1) {
        DD gridSize(pm.getGridSize());
        return (((ordinate / gridSize) + DD(0.5)).floor() * gridSize).ToDouble();
    }
    DD scale(pm.getScale());
    return (((ordinate * scale) + DD(0.5)).floor() / scale).ToDouble();
}

CoordinateXY
roundedIntersection(const CoordinateXY& p0, const CoordinateXY& p1,
                    const CoordinateXY& q0, const CoordinateXY& q1,
                    const geos::geom::PrecisionModel& pm)
{
    using geos::math::DD;
    DD rx = DD(p1.x) - DD(p0.x);
    DD ry = DD(p1.y) - DD(p0.y);
    DD sx = DD(q1.x) - DD(q0.x);
    DD sy = DD(q1.y) - DD(q0.y);
    DD qpx = DD(q0.x) - DD(p0.x);
    DD qpy = DD(q0.y) - DD(p0.y);
    DD denominator = rx * sy - ry * sx;
    if (denominator.isZero()) {
        return CoordinateXY::getNull();
    }
    DD t = (qpx * sy - qpy * sx) / denominator;
    DD x = DD(p0.x) + t * rx;
    DD y = DD(p0.y) + t * ry;
    if (x < DD(std::max(std::min(p0.x, p1.x), std::min(q0.x, q1.x))) ||
        x > DD(std::min(std::max(p0.x, p1.x), std::max(q0.x, q1.x))) ||
        y < DD(std::max(std::min(p0.y, p1.y), std::min(q0.y, q1.y))) ||
        y > DD(std::min(std::max(p0.y, p1.y), std::max(q0.y, q1.y)))) {
        return CoordinateXY::getNull();
    }
    return {roundIntersectionOrdinate(x, pm),
            roundIntersectionOrdinate(y, pm)};
}

}

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
                CoordinateXYZM intPt(li.getIntersection(intIndex));
                if (li.isProper()) {
                    // Round the intersection before converting it to double.
                    // Conversion can move a near-half-grid
                    // intersection into the wrong hot pixel.
                    CoordinateXY rounded = roundedIntersection(
                        seq0.getAt<CoordinateXY>(segIndex0),
                        seq0.getAt<CoordinateXY>(segIndex0 + 1),
                        seq1.getAt<CoordinateXY>(segIndex1),
                        seq1.getAt<CoordinateXY>(segIndex1 + 1),
                        *pm);
                    if (!rounded.isNull()) {
                        intPt.x = coordinateInCell(intPt.x, rounded.x, *pm);
                        intPt.y = coordinateInCell(intPt.y, rounded.y, *pm);
                    }
                }
                intersections.add(intPt);
                static_cast<NodedSegmentString*>(e0)->addIntersection(intPt, segIndex0);
                static_cast<NodedSegmentString*>(e1)->addIntersection(intPt, segIndex1);
            }
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
