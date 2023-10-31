/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/MCIndexSegmentSetMutualIntersector.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>
#include <geos/index/chain/MonotoneChainOverlapAction.h>

// std
#include <cstddef>

using namespace geos::index::chain;

namespace geos {
namespace noding { // geos::noding


/*private*/
void
MCIndexSegmentSetMutualIntersector::addChains(const SegmentString* segStr, MonoChains& chains) const
{
    if (segStr->size() == 0)
        return;
    MonotoneChainBuilder::getChains(segStr->getCoordinates(),
                                    (void*) segStr, overlapTolerance, chains);
}


/*private*/
void
MCIndexSegmentSetMutualIntersector::intersectChains(const MonoChains& chains, SegmentIntersector& segmentIntersector)
{
    MCIndexSegmentSetMutualIntersector::SegmentOverlapAction overlapAction(segmentIntersector);

    for(auto& queryChain : chains) {
        index.query(queryChain.getEnvelope(), [&queryChain, &overlapAction, &segmentIntersector, this](const MonotoneChain* testChain) -> bool {
            queryChain.computeOverlaps(testChain, overlapTolerance, &overlapAction);

            return !segmentIntersector.isDone(); // abort early if segmentIntersector->isDone()
        });
    }
}


/* public */
void
MCIndexSegmentSetMutualIntersector::setBaseSegments(SegmentString::ConstVect* segStrings)
{
    for(const SegmentString* css: *segStrings) {
        addChains(css, indexChains);
    }
}

/*public*/
void
MCIndexSegmentSetMutualIntersector::process(SegmentString::ConstVect* segStrings)
{
    process(segStrings, segInt);
}

/*public*/
void
MCIndexSegmentSetMutualIntersector::process(SegmentString::ConstVect* segStrings,
                                            SegmentIntersector* segmentIntersector)
{
    std::call_once(indexBuilt, [this]() {
        for (auto& mc: indexChains) {
            index.insert(&(mc.getEnvelope()), &mc);
        }
    });

    // TODO: Rework MonotoneChain extraction to take a callback, so we can pass the chains
    // to intersectChains as they are identified.
    MonoChains monoChains;
    for(const SegmentString* css: *segStrings) {
        SegmentString* ss = const_cast<SegmentString*>(css);
        addChains(ss, monoChains);
    }

    intersectChains(monoChains, *segmentIntersector);
}


/* public */
void
MCIndexSegmentSetMutualIntersector::SegmentOverlapAction::overlap(
    const MonotoneChain& mc1, std::size_t start1, const MonotoneChain& mc2, std::size_t start2)
{
    SegmentString* ss1 = static_cast<SegmentString*>(mc1.getContext());
    SegmentString* ss2 = static_cast<SegmentString*>(mc2.getContext());

    si.processIntersections(ss1, start1, ss2, start2);
}

} // geos::noding
} // geos
