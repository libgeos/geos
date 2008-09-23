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
	vector<int> startIndex;
	getChainStartIndices(pts, startIndex);
	size_t nindexes = startIndex.size();
	if (nindexes > 0)
	{
		size_t n = nindexes - 1;
		for(size_t i = 0; i < n; i++)
		{
			MonotoneChain* mc = new MonotoneChain(pts, startIndex[i], startIndex[i+1], context);
			mcList.push_back(mc);
		}
	}
}

/**
 * Return an array containing lists of start/end indexes of the monotone chains
 * for the given list of coordinates.
 * The last entry in the array points to the end point of the point array,
 * for use as a sentinel.
 */
void
MonotoneChainBuilder::getChainStartIndices(const CoordinateSequence *pts,
                                           vector<int>& startIndexList)
{
	// find the startpoint (and endpoints) of all monotone chains
	// in this edge
	int start = 0;
	startIndexList.push_back(start);
    const std::size_t n = pts->getSize() - 1;
	do
    {
		int last = findChainEnd(pts, start);
		startIndexList.push_back(last);
		start = last;
	} while (static_cast<std::size_t>(start) < n);

}

/**
 * @return the index of the last point in the monotone chain starting
 * at <code>start</code>.
 */
int
MonotoneChainBuilder::findChainEnd(const CoordinateSequence *pts, int start)
{
    std::size_t npts = pts->getSize();

	// skip any zero-length segments at start of sequence
	while ( pts->getAt( start).equals2D( pts->getAt( start + 1) ) )
		if ( ++start >= ( npts - 1 ) )
			// bail if there are no non-zero-length segments
			return npts - 1;
			
	// determine quadrant for chain
	int chainQuad=Quadrant::quadrant(pts->getAt(start),pts->getAt(start + 1));
	std::size_t last=start+1;
	while (last < npts)
	{
		// skip a zero-length segnments
		if (! pts->getAt( last - 1).equals2D( pts->getAt( last) ) )
		{
			// compute quadrant for next possible segment in chain
			int quad=Quadrant::quadrant(pts->getAt(last-1),pts->getAt(last));
			if (quad!=chainQuad) break;
		}
		last++;	
	}
#if GEOS_DEBUG
	std::cerr<<"MonotoneChainBuilder::findChainEnd() returning"<<std::endl;
#endif
	return static_cast<int>(last - 1);
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

