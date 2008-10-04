/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
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
#include <geos/noding/SegmentSetMutualIntersector.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/index/SpatialIndex.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>
#include <geos/index/chain/MonotoneChainOverlapAction.h>
#include <geos/index/strtree/STRtree.h>
// std
#include <cstddef>

using namespace geos::index::chain;

namespace geos {
namespace noding { // geos::noding
//
// private:
//

void 
MCIndexSegmentSetMutualIntersector::addToIndex(SegmentString* segStr)
{
    std::vector<MonotoneChain*>* segChains = 0;
    segChains = MonotoneChainBuilder::getChains(segStr->getCoordinates(), segStr);

    chainStore.push_back(segChains);

    for (std::size_t i = 0, n = segChains->size(); i < n; i++)
    {
        MonotoneChain * mc = (*segChains)[i];
        mc->setId(indexCounter++);
        index->insert(mc->getEnvelope(), mc);
    }
}

void 
MCIndexSegmentSetMutualIntersector::intersectChains()
{
    MCIndexSegmentSetMutualIntersector::SegmentOverlapAction overlapAction( *segInt);

    for (std::size_t i = 0, ni = monoChains->size(); i < ni; i++)
    {
        MonotoneChain * queryChain = (MonotoneChain *)((*monoChains)[i]);

        std::vector<void*> overlapChains;
        index->query( queryChain->getEnvelope(), overlapChains);

        for (std::size_t j = 0, nj = overlapChains.size(); j < nj; j++)
        {
            MonotoneChain * testChain = (MonotoneChain *)(overlapChains[j]);

            queryChain->computeOverlaps( testChain, &overlapAction);
            nOverlaps++;
            if (segInt->isDone()) 
                return;
        }
    }
}

void 
MCIndexSegmentSetMutualIntersector::addToMonoChains(SegmentString* segStr)
{
    std::vector<MonotoneChain*>* segChains = 0; 
    segChains = MonotoneChainBuilder::getChains(segStr->getCoordinates(), segStr);

    chainStore.push_back(segChains);

    for (std::size_t i = 0, ni = segChains->size(); i < ni; i++)
    {
        MonotoneChain* mc = (*segChains)[i];
        mc->setId( processCounter++ );
        monoChains->push_back(mc);
    }
}

//
// public:
//

MCIndexSegmentSetMutualIntersector::MCIndexSegmentSetMutualIntersector() 
:	monoChains( new std::vector<index::chain::MonotoneChain *>()),
index(new geos::index::strtree::STRtree()),
indexCounter(0),
processCounter(0),
nOverlaps(0)
{
}

MCIndexSegmentSetMutualIntersector::~MCIndexSegmentSetMutualIntersector() 
{
    delete index;
    delete monoChains;

    chainstore_mm_type::iterator end = chainStore.end();
    for (chainstore_mm_type::iterator it = chainStore.begin(); it != end; ++it)
    {
        typedef std::vector<index::chain::MonotoneChain*> chainstore_type;
        chainstore_type::iterator csend = (*it)->end();
        for (chainstore_type::iterator csit = (*it)->begin(); csit != csend; ++csit)
        {
            delete *csit;
        }
        delete *it;
    } 
}

void 
MCIndexSegmentSetMutualIntersector::setBaseSegments(SegmentString::ConstVect* segStrings)
{
    // NOTE - mloskot: const qualifier is removed silently, dirty.

    for (std::size_t i = 0, n = segStrings->size(); i < n; i++)
    {
        const SegmentString* css = (*segStrings)[i];
        SegmentString* ss = const_cast<SegmentString*>(css);
        addToIndex(ss);
    }
}

void 
MCIndexSegmentSetMutualIntersector::process(SegmentString::ConstVect * segStrings)
{
    processCounter = indexCounter + 1;
    nOverlaps = 0;
    monoChains->clear();

    for (std::size_t i = 0, n = segStrings->size(); i < n; i++)
    {
        SegmentString * seg = (SegmentString *)((*segStrings)[i]);
        addToMonoChains( seg);
    }
    intersectChains();
}


void 
MCIndexSegmentSetMutualIntersector::SegmentOverlapAction::overlap( MonotoneChain * mc1, int start1, MonotoneChain * mc2, int start2)
{
    SegmentString * ss1 = (SegmentString *)(mc1->getContext());
    SegmentString * ss2 = (SegmentString *)(mc2->getContext());

    si.processIntersections(ss1, start1, ss2, start2);
}

} // geos::noding
} // geos

/**********************************************************************
 * $Log$
 *
 **********************************************************************/
