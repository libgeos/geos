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
#include <functional>
#include "geos/indexChain.h"
#include "geos/noding.h"

using namespace geos::index::chain;

namespace geos {
namespace noding { // geos.noding

/*public*/
void
MCIndexNoder::computeNodes(SegmentString::NonConstVect* inputSegStrings)
{
	nodedSegStrings = inputSegStrings;

	for_each(nodedSegStrings->begin(), nodedSegStrings->end(),
			bind1st(mem_fun(&MCIndexNoder::add), this));

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
		vector<void*> overlapChains;
		index.query(queryChain->getEnvelope(), overlapChains);
		for (vector<void*>::iterator
			j=overlapChains.begin(), jEnd=overlapChains.end();
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
	}
}

/*private*/
void
MCIndexNoder::add(SegmentString* segStr)
{
	vector<indexMonotoneChain*> segChains;

	// segChains will contain nelwy allocated indexMonotoneChain objects
	MonotoneChainBuilder::getChains(segStr->getCoordinates(),
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


} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/02/23 23:17:52  strk
 * - Coordinate::nullCoordinate made private
 * - Simplified Coordinate inline definitions
 * - LMGeometryComponentFilter definition moved to LineMerger.cpp file
 * - Misc cleanups
 *
 * Revision 1.7  2006/02/23 20:05:21  strk
 * Fixed bug in MCIndexNoder constructor making memory checker go crazy, more
 * doxygen-friendly comments, miscellaneous cleanups
 *
 * Revision 1.6  2006/02/23 11:54:20  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.5  2006/02/21 16:53:49  strk
 * MCIndexPointSnapper, MCIndexSnapRounder
 *
 * Revision 1.4  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.3  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.2  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.1  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 **********************************************************************/
