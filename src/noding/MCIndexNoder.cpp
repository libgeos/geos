/**********************************************************************
 * $Id: MCIndexNoder.cpp 2479 2009-05-06 17:42:58Z strk $
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
 * Last port: noding/MCIndexNoder.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/index/chain/MonotoneChain.h> 
#include <geos/index/chain/MonotoneChainBuilder.h> 

#include <cassert>
#include <functional>
#include <algorithm>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifndef GEOS_INLINE
# include <geos/noding/MCIndexNoder.inl>
#endif

using namespace std;
using namespace geos::index::chain;

namespace geos {
namespace noding { // geos.noding

/*public*/
void
MCIndexNoder::computeNodes(SegmentString::NonConstVect* inputSegStrings)
{
	nodedSegStrings = inputSegStrings;
	assert(nodedSegStrings);

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

	for (vector<MonotoneChain*>::iterator
			i=monoChains.begin(), iEnd=monoChains.end();
			i != iEnd;
			++i)
	{

		MonotoneChain* queryChain = *i;
		assert(queryChain);
		vector<void*> overlapChains;
		index.query(&(queryChain->getEnvelope()), overlapChains);
		for (vector<void*>::iterator
			j=overlapChains.begin(), jEnd=overlapChains.end();
			j != jEnd;
			++j)
		{
			MonotoneChain* testChain = static_cast<MonotoneChain*>(*j);
			assert(testChain);

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

			// short-circuit if possible
			if (segInt->isDone()) return;

		}
	}
}

/*private*/
void
MCIndexNoder::add(SegmentString* segStr)
{
	vector<MonotoneChain*> segChains;

	// segChains will contain nelwy allocated MonotoneChain objects
	MonotoneChainBuilder::getChains(segStr->getCoordinates(),
			segStr, segChains);

	for(vector<MonotoneChain*>::iterator
			it=segChains.begin(), iEnd=segChains.end();
			it!=iEnd; ++it)
	{
		MonotoneChain* mc = *it;
		assert(mc);

		mc->setId(idCounter++);
		index.insert(&(mc->getEnvelope()), mc);

		// MonotoneChain objects deletion delegated to destructor
		monoChains.push_back(mc);
	}
}

MCIndexNoder::~MCIndexNoder()
{
	for(vector<MonotoneChain*>::iterator
			i=monoChains.begin(), iEnd=monoChains.end();
			i!=iEnd; ++i)
	{
		assert(*i);
		delete *i;
	}
}

void
MCIndexNoder::SegmentOverlapAction::overlap(MonotoneChain& mc1, size_t start1,
		MonotoneChain& mc2, size_t start2)
{
	SegmentString* ss1 = const_cast<SegmentString*>(
		static_cast<const SegmentString *>(mc1.getContext())
		);
	assert(ss1);

	SegmentString* ss2 = const_cast<SegmentString*>(
		static_cast<const SegmentString *>(mc2.getContext())
		);
	assert(ss2);

	si.processIntersections(ss1, start1, ss2, start2);
}


} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.15  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.14  2006/03/22 18:12:32  strk
 * indexChain.h header split.
 *
 * Revision 1.13  2006/03/15 09:51:12  strk
 * streamlined headers usage
 *
 * Revision 1.12  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.11  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.10  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.9  2006/02/28 14:34:05  strk
 * Added many assertions and debugging output hunting for a bug in BufferOp
 *
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
