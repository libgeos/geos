/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
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
 * Last port: noding/MCIndexNoder.java rev. 1.4 (JTS-1.7)
 *
 **********************************************************************/

#include <cassert>
#include "geos/indexChain.h"
#include "geos/noding.h"

namespace geos {

/*public*/
void
MCIndexNoder::computeNodes(SegmentString::NonConstVect* inputSegStrings)
{
	nodedSegStrings = inputSegStrings;

	for (SegmentString::NonConstVect::iterator
			i=inputSegStrings->begin(), e=inputSegStrings->end();
			i!=e;
			++i)
	{
		add(*i);
	}

	intersectChains();
//cerr<<"MCIndexNoder: # chain overlaps = "<<nOverlaps<<endl;
}

/*private*/
void
MCIndexNoder::intersectChains()
{
	assert(segInt);
	SegmentOverlapAction overlapAction(*segInt);

	for (vector<indexMonotoneChain*>::iterator
			i=monoChains.begin(), iEnd=monoChains.end();
			i != iEnd;
			++i)
	{

		indexMonotoneChain* queryChain = *i;
		vector<void*>* overlapChains = \
				index.query(queryChain->getEnvelope());
		for (vector<void*>::iterator
			j=overlapChains->begin(), jEnd=overlapChains->end();
			j != jEnd;
			++j)
		{
			indexMonotoneChain* testChain = (indexMonotoneChain*)(*j);

			/**
			 * following test makes sure we only compare each
			 * pair of chains once and that we don't compare a
			 * chain to itself
			 */
			if (testChain->getId() > queryChain->getId()) {
				queryChain->computeOverlaps(testChain,
						&overlapAction);
				nOverlaps++;
			}

		}

		delete overlapChains;
	}
}

/*private*/
void
MCIndexNoder::add(const SegmentString* segStr)
{
	vector<indexMonotoneChain*> segChains;

	// segChains will contain nelwy allocated indexMonotoneChain objects
	MonotoneChainBuilder::getChains(segStr->getCoordinatesRO(),
			segStr, segChains);

	for(vector<indexMonotoneChain*>::iterator
			it=segChains.begin(), iEnd=segChains.end();
			it!=iEnd; ++it)
	{
		indexMonotoneChain* mc = *it;
		mc->setId(idCounter++);
		index.insert(mc->getEnvelope(), mc);

		// indexMonotoneChain objects deletion delegated to destructor
		monoChains.push_back(mc);
	}
}

MCIndexNoder::~MCIndexNoder()
{
	for(vector<indexMonotoneChain*>::iterator
			i=monoChains.begin(), iEnd=monoChains.end();
			i!=iEnd; ++i)
	{
		delete *i;
	}
}

void
MCIndexNoder::SegmentOverlapAction::overlap(indexMonotoneChain* mc1, int start1,
		indexMonotoneChain* mc2, int start2)
{
	SegmentString* ss1 = const_cast<SegmentString*>(
		static_cast<const SegmentString *>(mc1->getContext())
		);

	SegmentString* ss2 = const_cast<SegmentString*>(
		static_cast<const SegmentString *>(mc2->getContext())
		);

	si.processIntersections(ss1, start1, ss2, start2);
}


} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 **********************************************************************/
