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
 * Last port: noding/MCIndexNoder.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>
#include <geos/geom/Envelope.h>
#include <geos/util/Interrupt.h>

#include <cassert>
#include <functional>
#include <algorithm>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using geos::index::chain::MonotoneChain;
using geos::index::chain::MonotoneChainBuilder;

namespace geos {
namespace noding { // geos.noding

/*public*/
void
MCIndexNoder::computeNodes(SegmentString::NonConstVect* inputSegStrings)
{
    nodedSegStrings = inputSegStrings;
    assert(nodedSegStrings);

    for (const auto& s : *nodedSegStrings) {
        add(s);
    }

    if (!indexBuilt) {
        for(const auto& mc : monoChains) {
            index.insert(mc.getEnvelope(overlapTolerance), &mc);
        }
        indexBuilt = true;
    }

    intersectChains();
}


/*private*/
void
MCIndexNoder::intersectChains()
{
    assert(segInt);

    SegmentOverlapAction overlapAction(*segInt);

    index.queryPairs([this, &overlapAction](const MonotoneChain* queryChain, const MonotoneChain* testChain) {
        queryChain->computeOverlaps(testChain, overlapTolerance, &overlapAction);
        nOverlaps++;
        if ( nOverlaps % 100000 == 0 ) GEOS_CHECK_FOR_INTERRUPTS();

        return !segInt->isDone(); // abort early if segInt->isDone()
    });
}

/*private*/
void
MCIndexNoder::add(SegmentString* segStr)
{
    // std::vector<std::unique_ptr<MonotoneChain>> segChains;

    // segChains will contain newly allocated MonotoneChain objects
    MonotoneChainBuilder::getChains(segStr->getCoordinates(),
                                    segStr, monoChains);

}


void
MCIndexNoder::SegmentOverlapAction::overlap(const MonotoneChain& mc1, std::size_t start1,
        const MonotoneChain& mc2, std::size_t start2)
{
    SegmentString* ss1 = const_cast<SegmentString*>(
                             static_cast<const SegmentString*>(mc1.getContext())
                         );
    assert(ss1);

    SegmentString* ss2 = const_cast<SegmentString*>(
                             static_cast<const SegmentString*>(mc2.getContext())
                         );
    assert(ss2);

    si.processIntersections(ss1, start1, ss2, start2);
}


} // namespace geos.noding
} // namespace geos
