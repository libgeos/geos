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
                             size_t nstart, size_t nend, void* nContext)
    :
    pts(newPts),
    env(newPts[nstart], newPts[nend]),
    context(nContext),
    start(nstart),
    end(nend),
    id(-1)
{
}

const Envelope&
MonotoneChain::getEnvelope() const
{
    return env;
}

void
MonotoneChain::getLineSegment(size_t index, LineSegment& ls) const
{
    ls.p0 = pts[index];
    ls.p1 = pts[index + 1];
}

std::unique_ptr<CoordinateSequence>
MonotoneChain::getCoordinates() const
{
    return std::unique_ptr<CoordinateSequence>(pts.clone());
}

void
MonotoneChain::select(const Envelope& searchEnv, MonotoneChainSelectAction& mcs)
{
    computeSelect(searchEnv, start, end, mcs);
}

void
MonotoneChain::computeSelect(const Envelope& searchEnv,
                             size_t start0, size_t end0,
                             MonotoneChainSelectAction& mcs)
{
    const Coordinate& p0 = pts[start0];
    const Coordinate& p1 = pts[end0];

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
    size_t mid = (start0 + end0) / 2;

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
MonotoneChain::computeOverlaps(MonotoneChain* mc,
                               MonotoneChainOverlapAction* mco)
{
    computeOverlaps(start, end, *mc, mc->start, mc->end, *mco);
}

/*private*/
void
MonotoneChain::computeOverlaps(size_t start0, size_t end0,
                               MonotoneChain& mc,
                               size_t start1, size_t end1,
                               MonotoneChainOverlapAction& mco)
{
    // terminating condition for the recursion
    if(end0 - start0 == 1 && end1 - start1 == 1) {
        mco.overlap(*this, start0, mc, start1);
        return;
    }

    // nothing to do if the envelopes of these subchains don't overlap
    if(!overlaps(start0, end0, mc, start1, end1)) {
        return;
    }

    // the chains overlap,so split each in half and iterate (binary search)
    size_t mid0 = (start0 + end0) / 2;
    size_t mid1 = (start1 + end1) / 2;

    // Assert: mid != start or end (since we checked above for
    // end-start <= 1)
    // check terminating conditions before recursing
    if(start0 < mid0) {
        if(start1 < mid1) {
            computeOverlaps(start0, mid0, mc, start1, mid1, mco);
        }
        if(mid1 < end1) {
            computeOverlaps(start0, mid0, mc, mid1, end1, mco);
        }
    }

    if(mid0 < end0) {
        if(start1 < mid1) {
            computeOverlaps(mid0, end0, mc, start1, mid1, mco);
        }
        if(mid1 < end1) {
            computeOverlaps(mid0, end0, mc, mid1, end1, mco);
        }
    }
}

/*private*/
bool
MonotoneChain::overlaps(size_t start0, size_t end0, const MonotoneChain& mc, size_t start1, size_t end1)
{
    return Envelope::intersects(pts.getAt(start0), pts.getAt(end0),
                                mc.pts.getAt(start1), mc.pts.getAt(end1));
}

} // namespace geos.index.chain
} // namespace geos.index
} // namespace geos
