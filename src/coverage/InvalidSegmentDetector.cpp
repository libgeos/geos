/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/InvalidSegmentDetector.h>
#include <geos/coverage/CoverageRing.h>

#include <geos/algorithm/PolygonNodeTopology.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/noding/SegmentString.h>


using geos::algorithm::PolygonNodeTopology;
using geos::algorithm::LineIntersector;
using geos::geom::Coordinate;
using geos::geom::LineSegment;
using geos::noding::SegmentString;


namespace geos {     // geos
namespace coverage { // geos.coverage


/* public */
void
InvalidSegmentDetector::processIntersections(
    SegmentString* ssAdj,    std::size_t iAdj,
    SegmentString* ssTarget, std::size_t iTarget)
{
    // note the source of the edges is important
    CoverageRing* adj = static_cast<CoverageRing*>(ssAdj);
    CoverageRing* target = static_cast<CoverageRing*>(ssTarget);

    //-- skip target segments with known status
    if (target->isKnown(iTarget)) return;

    const Coordinate& t0 = target->getCoordinate(iTarget);
    const Coordinate& t1 = target->getCoordinate(iTarget + 1);
    const Coordinate& adj0 = adj->getCoordinate(iAdj);
    const Coordinate& adj1 = adj->getCoordinate(iAdj + 1);

    //-- skip zero-length segments
    if (t0.equals2D(t1) || adj0.equals2D(adj1))
        return;

    if (isEqual(t0, t1, adj0, adj1))
        return;

    bool bInvalid = isInvalid(t0, t1, adj0, adj1, adj, iAdj);
    if (bInvalid) {
        target->markInvalid(iTarget);
    }
}


/* private */
bool
InvalidSegmentDetector::isEqual(
    const Coordinate& t0, const Coordinate& t1,
    const Coordinate& adj0, const Coordinate& adj1)
{
    if (t0.equals2D(adj0) && t1.equals2D(adj1))
        return true;
    if (t0.equals2D(adj1) && t1.equals2D(adj0))
        return true;
    return false;
}


/* private */
bool
InvalidSegmentDetector::isInvalid(const Coordinate& tgt0, const Coordinate& tgt1,
    const Coordinate& adj0, const Coordinate& adj1,
    CoverageRing* adj, std::size_t indexAdj)
{
    //-- segments that are collinear (but not matching) or are interior are invalid
    if (isCollinearOrInterior(tgt0, tgt1, adj0, adj1, adj, indexAdj))
        return true;

    //-- segments which are nearly parallel for a significant length are invalid
    if (distanceTol > 0 && isNearlyParallel(tgt0, tgt1, adj0, adj1, distanceTol))
        return true;

    return false;
}


/* private */
bool
InvalidSegmentDetector::isCollinearOrInterior(
    const Coordinate& tgt0, const Coordinate& tgt1,
    const Coordinate& adj0, const Coordinate& adj1,
    CoverageRing* adj, std::size_t indexAdj)
{
    LineIntersector li;
    li.computeIntersection(tgt0, tgt1, adj0, adj1);

    //-- segments do not interact
    if (! li.hasIntersection())
        return false;

    //-- If the segments are collinear, they do not match, so are invalid.
    if (li.getIntersectionNum() == 2) {
        //TODO: assert segments are not equal?
        return true;
    }

    //-- target segment crosses, or segments touch at non-endpoint
    if (li.isProper() || li.isInteriorIntersection()) {
        return true;
    }

    /**
     * At this point the segments have a single intersection point
     * which is an endpoint of both segments.
     *
     * Check if the target segment lies in the interior of the adj ring.
     */
    const Coordinate& intVertex = li.getIntersection(0);
    bool isInterior = isInteriorSegment(intVertex, tgt0, tgt1, adj, indexAdj);
    return isInterior;
}


/* private */
bool
InvalidSegmentDetector::isInteriorSegment(
    const Coordinate& intVertex,
    const Coordinate& tgt0, const Coordinate& tgt1,
    CoverageRing* adj, std::size_t indexAdj)
{
    //-- find target segment endpoint which is not the intersection point
    const Coordinate* tgtEnd = intVertex.equals2D(tgt0) ? &tgt1 : &tgt0;

    //-- find adjacent-ring vertices on either side of intersection vertex
    const Coordinate* adjPrev = &adj->findVertexPrev(indexAdj, intVertex);
    const Coordinate* adjNext = &adj->findVertexNext(indexAdj, intVertex);

    //-- don't check if test segment is equal to either corner segment
    if (tgtEnd->equals2D(*adjPrev) || tgtEnd->equals2D(*adjNext)) {
        return false;
    }

    //-- if needed, re-orient corner to have interior on right
    if (! adj->isInteriorOnRight()) {
        const Coordinate* temp = adjPrev;
        adjPrev = adjNext;
        adjNext = temp;
    }

    bool isInterior = PolygonNodeTopology::isInteriorSegment(&intVertex, adjPrev, adjNext, tgtEnd);
    return isInterior;
}


/* private static */
bool
InvalidSegmentDetector::isNearlyParallel(
    const Coordinate& p00, const Coordinate& p01,
    const Coordinate& p10, const Coordinate& p11,
    double distanceTol)
{
    LineSegment line0(p00, p01);
    LineSegment line1(p10, p11);
    LineSegment proj0;
    LineSegment proj1;

    if (!line0.project(line1, proj0))
        return false;

    if (!line1.project(line0, proj1))
        return false;

    if (proj0.getLength() <= distanceTol || proj1.getLength() <= distanceTol)
        return false;

    if (proj0.p0.distance(proj1.p1) < proj0.p0.distance(proj1.p0)) {
        proj1.reverse();
    }
    return proj0.p0.distance(proj1.p0) <= distanceTol
        && proj0.p1.distance(proj1.p1) <= distanceTol;
}



} // namespace geos.coverage
} // namespace geos


