/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Geometry.h>
#include <geos/geom/Coordinate.h>
#include <geos/operation/relateng/EdgeSegmentIntersector.h>
#include <geos/operation/relateng/RelateSegmentString.h>
#include <geos/operation/relateng/TopologyComputer.h>
#include <geos/operation/relateng/NodeSection.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>


using geos::geom::CoordinateXYZM;
using geos::geom::CoordinateXY;
using geos::geom::Geometry;
using geos::noding::SegmentString;
using geos::index::chain::MonotoneChain;
using geos::index::chain::MonotoneChainBuilder;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public override */
bool
EdgeSegmentIntersector::isDone() const
{
    return topoComputer.isResultKnown();
}


/* public override */
void
EdgeSegmentIntersector::processIntersections(
    SegmentString* ss0, std::size_t segIndex0,
    SegmentString* ss1, std::size_t segIndex1)
{
    // don't intersect a segment with itself
    if (ss0 == ss1 && segIndex0 == segIndex1)
        return;

    RelateSegmentString* rss0 = static_cast<RelateSegmentString*>(ss0);
    RelateSegmentString* rss1 = static_cast<RelateSegmentString*>(ss1);
    //TODO: move this ordering logic to TopologyBuilder
    if (rss0->isA()) {
        addIntersections(rss0, segIndex0, rss1, segIndex1);
    }
    else {
        addIntersections(rss1, segIndex1, rss0, segIndex0);
    }
}


/* private */
void
EdgeSegmentIntersector::addIntersections(
    RelateSegmentString* ssA, std::size_t segIndexA,
    RelateSegmentString* ssB, std::size_t segIndexB)
{

    const CoordinateXY& a0 = ssA->getCoordinate(segIndexA);
    const CoordinateXY& a1 = ssA->getCoordinate(segIndexA + 1);
    const CoordinateXY& b0 = ssB->getCoordinate(segIndexB);
    const CoordinateXY& b1 = ssB->getCoordinate(segIndexB + 1);

    li.computeIntersection(a0, a1, b0, b1);

    if (! li.hasIntersection())
        return;

    for (uint32_t i = 0; i < li.getIntersectionNum(); i++)
    {
        const CoordinateXYZM& intPtXYZM = li.getIntersection(i);
        CoordinateXY intPt(intPtXYZM.x, intPtXYZM.y);
        /**
         * Ensure endpoint intersections are added once only, for their canonical segments.
         * Proper intersections lie on a unique segment so do not need to be checked.
         * And it is important that the Containing Segment check not be used,
         * since due to intersection computation roundoff,
         * it is not reliable in that situation.
         */
        if (li.isProper() ||
            (ssA->isContainingSegment(segIndexA, &intPt) &&
             ssB->isContainingSegment(segIndexB, &intPt)))
        {
            NodeSection* nsa = ssA->createNodeSection(segIndexA, intPt);
            NodeSection* nsb = ssB->createNodeSection(segIndexB, intPt);
            topoComputer.addIntersection(nsa, nsb);
        }
    }
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


