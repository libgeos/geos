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
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Distance.h>
#include <geos/math/DD.h>

#include <algorithm>
#include <cmath>


using namespace geos::algorithm;
using namespace geos::geom;
using geos::math::DD;

namespace {

/*
 * Returns an intersection ordinate that the precision model rounds to
 * cellValue: the ordinate itself, or the adjacent double towards cellValue
 * if that one is rounded to cellValue and lies in [rangeMin, rangeMax].
 * Otherwise the ordinate is returned unchanged.
 *
 * One step is enough whenever the ordinate is the nearest double to the
 * double-double intersection ordinate, and cellValue is the grid value
 * of that double-double ordinate. The adjacent double towards it then
 * lies at or beyond the double-double ordinate, so in exact arithmetic it
 * is in the same cell. The step is still confirmed with makePrecise,
 * because makePrecise computes in double: a double next to a half-cell
 * boundary can be scaled onto the boundary and rounded into the other cell.
 * The point is then kept rather than moved further. Each further step
 * would move the node further from the exact intersection, and at grids
 * of about 14 significant digits the noder's nearness tolerance, a
 * hundredth of a cell, is about one representable double.
 *
 * The range is where both segments overlap in this ordinate.
 * makePrecise is non-decreasing, so an ordinate in the range of a segment
 * is rounded to a value between the rounded ordinates of its vertices.
 * Holding for each ordinate, this keeps the moved point within the
 * envelope of both segments, the invariant that LineIntersector enforces
 * with isInSegmentEnvelopes, and the rounded intersection within the
 * envelope of each rounded segment.
 * For a horizontal or vertical segment the range is a single value, and
 * the intersection keeps the segment's own ordinate.
 */
double
ordinateInCell(double ordinate, double cellValue,
               double rangeMin, double rangeMax,
               const PrecisionModel& pm)
{
    if (pm.makePrecise(ordinate) == cellValue) {
        return ordinate;
    }
    double adjacent = std::nextafter(ordinate, cellValue);
    if (adjacent < rangeMin || adjacent > rangeMax) {
        return ordinate;
    }
    return pm.makePrecise(adjacent) == cellValue ? adjacent : ordinate;
}

/*
 * Moves a proper intersection point, as computed by LineIntersector for
 * the segments p0-p1 and q0-q1, into the grid cell of the double-double
 * intersection point, where one representable double per ordinate is
 * enough (see ordinateInCell). Z and M are not changed.
 *
 * LineIntersector computes the point with Intersection::intersection,
 * which returns the nearest doubles to CGAlgorithmsDD::intersectionDD.
 * LineIntersector substitutes a segment endpoint instead if the lines are
 * parallel in double-double arithmetic, or if the rounded point falls
 * outside a segment envelope (isInSegmentEnvelopes). Such a point is not
 * moved. The check for the second case is defensive: no input is known
 * to reach it for a proper intersection.
 */
void
moveIntoCell(CoordinateXYZM& intPt,
             const CoordinateXY& p0, const CoordinateXY& p1,
             const CoordinateXY& q0, const CoordinateXY& q1,
             const PrecisionModel& pm)
{
    DD x;
    DD y;
    if (!CGAlgorithmsDD::intersectionDD(p0, p1, q0, q1, x, y)) {
        return;
    }
    // Defensive: no input is known to reach this for a proper intersection.
    if (x.ToDouble() != intPt.x || y.ToDouble() != intPt.y) {
        return;
    }
    intPt.x = ordinateInCell(intPt.x, pm.makePrecise(x),
                             std::max(std::min(p0.x, p1.x), std::min(q0.x, q1.x)),
                             std::min(std::max(p0.x, p1.x), std::max(q0.x, q1.x)),
                             pm);
    intPt.y = ordinateInCell(intPt.y, pm.makePrecise(y),
                             std::max(std::min(p0.y, p1.y), std::min(q0.y, q1.y)),
                             std::min(std::max(p0.y, p1.y), std::max(q0.y, q1.y)),
                             pm);
}

} // anonymous namespace

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
                // The nearest double to a proper intersection can lie in a
                // different grid cell than the intersection itself.
                // The same point is used for the hot pixel and both segment nodes.
                if (li.isProper()) {
                    moveIntoCell(intPt,
                                 seq0.getAt<CoordinateXY>(segIndex0),
                                 seq0.getAt<CoordinateXY>(segIndex0 + 1),
                                 seq1.getAt<CoordinateXY>(segIndex1),
                                 seq1.getAt<CoordinateXY>(segIndex1 + 1),
                                 pm);
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
