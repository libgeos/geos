/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: index/chain/MonotoneChain.java rev. 1.15 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainSelectAction.h>
#include <geos/index/chain/MonotoneChainOverlapAction.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/Envelope.h>
#include <geos/util.h>

using namespace geos::geom;

namespace geos {
namespace index { // geos.index
namespace chain { // geos.index.chain

MonotoneChain::MonotoneChain(const geom::CoordinateSequence& newPts,
                             std::size_t nstart, std::size_t nend, void* nContext)
    : pts(&newPts)
    , context(nContext)
    , start(nstart)
    , end(nend)
    , env()
{}

const Envelope&
MonotoneChain::getEnvelope() const
{
    return getEnvelope(0.0);
}

const Envelope&
MonotoneChain::getEnvelope(double expansionDistance) const
{
    if (env.isNull()) {
        env.init(pts->getAt<CoordinateXY>(start), pts->getAt<CoordinateXY>(end));
        if (expansionDistance > 0.0) {
            env.expandBy(expansionDistance);
        }
    }
    return env;
}

std::unique_ptr<CoordinateSequence>
MonotoneChain::getCoordinates() const
{
    return pts->clone();
}

void
MonotoneChain::select(const Envelope& searchEnv, MonotoneChainSelectAction& mcs) const
{
    computeSelect(searchEnv, start, end, mcs);
}

void
MonotoneChain::computeSelect(const Envelope& searchEnv,
                             std::size_t start0, std::size_t end0,
                             MonotoneChainSelectAction& mcs) const
{
    const CoordinateXY& p0 = pts->getAt<CoordinateXY>(start0);
    const CoordinateXY& p1 = pts->getAt<CoordinateXY>(end0);

    // terminating condition for the recursion
    if(end0 - start0 == 1) {
        mcs.select(*this, start0);
        return;
    }
    // nothing to do if the envelopes don't overlap
    if(!searchEnv.intersects(p0, p1)) {
        return;
    }
    // the chains overlap,so split each in half and iterate (binary search)
    std::size_t mid = (start0 + end0) / 2;

    // Assert: mid != start or end (since we checked above for end-start <= 1)
    // check terminating conditions before recursing
    if(start0 < mid) {
        computeSelect(searchEnv, start0, mid, mcs);
    }

    if(mid < end0) {
        computeSelect(searchEnv, mid, end0, mcs);
    }
}

/* public */
void
MonotoneChain::computeOverlaps(const MonotoneChain* mc,
                               MonotoneChainOverlapAction* mco) const
{
    computeOverlaps(start, end, *mc, mc->start, mc->end, 0.0, *mco);
}

/* public */
void
MonotoneChain::computeOverlaps(const MonotoneChain* mc, double overlapTolerance,
                               MonotoneChainOverlapAction* mco) const
{
    computeOverlaps(start, end, *mc, mc->start, mc->end, overlapTolerance, *mco);
}

/*private*/
void
MonotoneChain::computeOverlaps(std::size_t start0, std::size_t end0,
                               const MonotoneChain& mc,
                               std::size_t start1, std::size_t end1,
                               double overlapTolerance,
                               MonotoneChainOverlapAction& mco) const
{
    // terminating condition for the recursion
    if(end0 - start0 == 1 && end1 - start1 == 1) {
        mco.overlap(*this, start0, mc, start1);
        return;
    }

    // nothing to do if the envelopes of these subchains don't overlap
    if(!overlaps(start0, end0, mc, start1, end1, overlapTolerance)) {
        return;
    }

    // the chains overlap,so split each in half and iterate (binary search)
    std::size_t mid0 = (start0 + end0) / 2;
    std::size_t mid1 = (start1 + end1) / 2;

    // Assert: mid != start or end (since we checked above for
    // end-start <= 1)
    // check terminating conditions before recursing
    if(start0 < mid0) {
        if(start1 < mid1) {
            computeOverlaps(start0, mid0, mc, start1, mid1, overlapTolerance, mco);
        }
        if(mid1 < end1) {
            computeOverlaps(start0, mid0, mc, mid1, end1, overlapTolerance, mco);
        }
    }

    if(mid0 < end0) {
        if(start1 < mid1) {
            computeOverlaps(mid0, end0, mc, start1, mid1, overlapTolerance, mco);
        }
        if(mid1 < end1) {
            computeOverlaps(mid0, end0, mc, mid1, end1, overlapTolerance, mco);
        }
    }
}

/*private*/
bool
MonotoneChain::overlaps(const CoordinateXY& p1, const CoordinateXY& p2,
                        const CoordinateXY& q1, const CoordinateXY& q2,
                        double overlapTolerance)
{
    double maxq = std::max(q1.x, q2.x);
    double minp = std::min(p1.x, p2.x);
    if (minp > (maxq + overlapTolerance))
        return false;

    double minq = std::min(q1.x, q2.x);
    double maxp = std::max(p1.x, p2.x);
    if (maxp < (minq - overlapTolerance))
        return false;

    maxq = std::max(q1.y, q2.y);
    minp = std::min(p1.y, p2.y);
    if (minp > (maxq + overlapTolerance))
        return false;

    minq = std::min(q1.y, q2.y);
    maxp = std::max(p1.y, p2.y);
    if (maxp < (minq - overlapTolerance))
        return false;

    return true;
}


} // namespace geos.index.chain
} // namespace geos.index
} // namespace geos
