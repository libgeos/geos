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
 **********************************************************************
 *
 * Last port: index/chain/MonotoneChainBuilder.java rev 1.12 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/index/chain/MonotoneChainBuilder.h> 
#include <geos/index/chain/MonotoneChain.h> 
#include <geos/geom/CoordinateSequence.h>
#include <geos/geomgraph/Quadrant.h>

#include <cstdio>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

using namespace std;
using namespace geos::geomgraph;
using namespace geos::geom;

namespace geos {
namespace index { // geos.index
namespace chain { // geos.index.chain

/* static public */
vector<MonotoneChain*>*
MonotoneChainBuilder::getChains(const CoordinateSequence* pts, void* context)
{
	vector<MonotoneChain*>* mcList = new vector<MonotoneChain*>();
	getChains(pts, context, *mcList);
	return mcList;
}

/* static public */
void
MonotoneChainBuilder::getChains(const CoordinateSequence* pts, void* context,
                                vector<MonotoneChain*>& mcList)
{
	vector<size_t> startIndex;
	getChainStartIndices(*pts, startIndex);
	size_t nindexes = startIndex.size();
	if (nindexes > 0)
	{
		size_t n = nindexes - 1;
		for(size_t i = 0; i < n; i++)
		{
			MonotoneChain* mc = new MonotoneChain(*pts, startIndex[i], startIndex[i+1], context);
			mcList.push_back(mc);
		}
	}
}

/* static public */
void
MonotoneChainBuilder::getChainStartIndices(const CoordinateSequence& pts,
                                           vector<size_t>& startIndexList)
{
	// find the startpoint (and endpoints) of all monotone chains
	// in this edge
	size_t start = 0;
	startIndexList.push_back(start);
	const size_t n = pts.getSize() - 1;
	do
	{
		size_t last = findChainEnd(pts, start);
		startIndexList.push_back(last);
		start = last;
	} while (start < n);

}

/* private static */
size_t
MonotoneChainBuilder::findChainEnd(const CoordinateSequence& pts, size_t start)
{

	const size_t npts = pts.getSize(); // cache

	assert(start < npts);
	assert(npts); // should be implied by the assertion above,
	              // 'start' being unsigned

	size_t safeStart = start;

        // skip any zero-length segments at the start of the sequence
        // (since they cannot be used to establish a quadrant)
	while ( safeStart < npts - 1
		&& pts[safeStart].equals2D(pts[safeStart+1]) ) 
	{
		++safeStart;
	}

	// check if there are NO non-zero-length segments
	if (safeStart >= npts - 1) {
		return npts - 1;
	}

	// determine overall quadrant for chain
	// (which is the starting quadrant)
	int chainQuad = Quadrant::quadrant(pts[safeStart],
	                                   pts[safeStart + 1]);
	size_t last = start + 1;
	while (last < npts)
	{
		// skip zero-length segments, but include them in the chain
		if (! pts[last - 1].equals2D( pts[last] ) )
		{
			// compute quadrant for next possible segment in chain
			int quad = Quadrant::quadrant( pts[last - 1],
			                               pts[last]      );
			if (quad != chainQuad) break;
		}
		++last;	
	}
#if GEOS_DEBUG
	std::cerr<<"MonotoneChainBuilder::findChainEnd() returning"<<std::endl;
#endif

	return last - 1;
}

} // namespace geos.index.chain
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.25  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.24  2006/03/23 13:31:58  strk
 * Fixed to allow build with GEOS_DEBUG
 *
 * Revision 1.23  2006/03/22 18:12:32  strk
 * indexChain.h header split.
 *
 **********************************************************************/

