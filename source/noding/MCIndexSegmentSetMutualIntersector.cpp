/**********************************************************************
 * $Id:
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

using namespace geos::index::chain;

namespace geos {
namespace noding { // geos::noding
//
// private:
//

void 
MCIndexSegmentSetMutualIntersector::addToIndex( SegmentString * segStr)
{
	std::vector<MonotoneChain *> * segChains; 
	segChains = MonotoneChainBuilder::getChains( segStr->getCoordinates(), segStr);

	for (size_t i=0, n=segChains->size(); i<n; i++)
	{
		MonotoneChain * mc = (*segChains)[ i ];
		mc->setId(indexCounter++);
		index->insert(mc->getEnvelope(), mc);
	}

// BWJ - seems to cause some tests of prepared predicates to fail, but leaving this out probably causes a 
// memory leak.  more research needed
//	for ( std::vector<MonotoneChain *>::iterator i = segChains->begin(), e = segChains->end(); i != e; i++ )
//		delete *i;
//	delete segChains;
}

void 
MCIndexSegmentSetMutualIntersector::intersectChains()
{
	MCIndexSegmentSetMutualIntersector::SegmentOverlapAction overlapAction( *segInt);

	for (size_t i = 0, ni = monoChains->size(); i < ni; i++)
	{
		MonotoneChain * queryChain = (MonotoneChain *)((*monoChains)[i]);
		
		std::vector<void*> overlapChains;
		index->query( queryChain->getEnvelope(), overlapChains);

		for (size_t j = 0, nj = overlapChains.size(); j < nj; j++)
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
MCIndexSegmentSetMutualIntersector::addToMonoChains( SegmentString * segStr)
{
	std::vector<MonotoneChain *> * segChains; 
	segChains = MonotoneChainBuilder::getChains( segStr->getCoordinates(), segStr);

	for (size_t i = 0, ni = segChains->size(); i < ni; i++)
	{
		MonotoneChain * mc = (*segChains)[ i ];
		mc->setId( processCounter++ );
		monoChains->push_back( mc);
	}

	delete segChains;
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
{ }

MCIndexSegmentSetMutualIntersector::~MCIndexSegmentSetMutualIntersector() 
{
	delete index;
	for ( int i = 0, ni = monoChains->size(); i < ni; i++ )
		delete (*monoChains)[ i ];
	delete monoChains;
}

void 
MCIndexSegmentSetMutualIntersector::setBaseSegments( SegmentString::ConstVect * segStrings)
{
	for (size_t i=0, n=segStrings->size(); i<n; i++)
	{
		SegmentString * ss = (SegmentString *)((*segStrings)[i]);
		addToIndex( ss);
	}
}

void 
MCIndexSegmentSetMutualIntersector::process( SegmentString::ConstVect * segStrings)
{
	processCounter = indexCounter + 1;
	nOverlaps = 0;
	monoChains->clear();

	for (size_t i=0, n=segStrings->size(); i<n; i++)
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
