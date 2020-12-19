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
 **********************************************************************
 *
 * Last port: noding/MCIndexSegmentSetMutualIntersector.java r388 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/noding/MCIndexSegmentSetMutualIntersector.h>
#include <geos/noding/SegmentSetMutualIntersector.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/index/SpatialIndex.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>
#include <geos/index/chain/MonotoneChainOverlapAction.h>
#include <geos/index/strtree/SimpleSTRtree.h>
// std
#include <cstddef>

using namespace geos::index::chain;

namespace geos {
namespace noding { // geos::noding

/*private*/
void
MCIndexSegmentSetMutualIntersector::addToIndex(SegmentString* segStr)
{
    MonoChains segChains = MonotoneChainBuilder::getChains(segStr->getCoordinates(), segStr);

    for(auto& mc : segChains) {
        //mc.setId(indexCounter++);
        index->insert(&(mc.getEnvelope()), &mc);
    }
    chainStore.push_back(std::move(segChains));
}


    /*private*/
void
MCIndexSegmentSetMutualIntersector::intersectChains()
{
    MCIndexSegmentSetMutualIntersector::SegmentOverlapAction overlapAction(*segInt);

    std::vector<void*> overlapChains;
    for(MonotoneChain& queryChain : monoChains) {
        overlapChains.clear();

        index->query(&(queryChain.getEnvelope()), overlapChains);

        for(void* hit : overlapChains) {
            MonotoneChain* testChain = static_cast<MonotoneChain*>(hit);

            queryChain.computeOverlaps(testChain, &overlapAction);
            nOverlaps++;
            if(segInt->isDone()) {
                return;
            }
        }
    }
}

/*private*/
void
MCIndexSegmentSetMutualIntersector::addToMonoChains(SegmentString* segStr)
{
    MonoChains segChains = MonotoneChainBuilder::getChains(segStr->getCoordinates(), segStr);

    MonoChains::size_type n = segChains.size();
    monoChains.reserve(monoChains.size() + n);
    for(auto& mc : segChains) {
        mc.setId(processCounter++);
        monoChains.push_back(mc);
    }
}

/* public */
MCIndexSegmentSetMutualIntersector::MCIndexSegmentSetMutualIntersector()
    :	monoChains(),
      index(new geos::index::strtree::SimpleSTRtree()),
      indexCounter(0),
      processCounter(0),
      nOverlaps(0)
{
}

/* public */
MCIndexSegmentSetMutualIntersector::~MCIndexSegmentSetMutualIntersector()
{
    delete index;
}

/* public */
void
MCIndexSegmentSetMutualIntersector::setBaseSegments(SegmentString::ConstVect* segStrings)
{
    // NOTE - mloskot: const qualifier is removed silently, dirty.
    chainStore.reserve(segStrings->size());
    for(const SegmentString* css : *segStrings) {
        SegmentString* ss = const_cast<SegmentString*>(css);
        addToIndex(ss);
    }
}

/*public*/
void
MCIndexSegmentSetMutualIntersector::process(SegmentString::ConstVect* segStrings)
{
    processCounter = indexCounter + 1;
    nOverlaps = 0;

    monoChains.clear();

    for(SegmentString::ConstVect::size_type i = 0, n = segStrings->size(); i < n; i++) {
        SegmentString* seg = (SegmentString*)((*segStrings)[i]);
        addToMonoChains(seg);
    }
    intersectChains();
}


/* public */
void
MCIndexSegmentSetMutualIntersector::SegmentOverlapAction::overlap(
    MonotoneChain& mc1, std::size_t start1, MonotoneChain& mc2, std::size_t start2)
{
    SegmentString* ss1 = (SegmentString*)(mc1.getContext());
    SegmentString* ss2 = (SegmentString*)(mc2.getContext());

    si.processIntersections(ss1, start1, ss2, start2);
}

} // geos::noding
} // geos

