/**********************************************************************
 * $Id: IteratedNoder.cpp 2319 2009-04-07 19:00:36Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/IteratedNoder.java rev. 1.6 (JTS-1.9)
 *
 **********************************************************************/

#include <sstream>
#include <vector>

#include <geos/profiler.h>
#include <geos/util/TopologyException.h> 
#include <geos/noding/IteratedNoder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/IntersectionAdder.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding

/*
 * Node the input segment strings once
 * and create the split edges between the nodes
 */
void
IteratedNoder::node(vector<SegmentString*> *segStrings,
		int *numInteriorIntersections)
{
	IntersectionAdder si(li);
	MCIndexNoder noder;
	noder.setSegmentIntersector(&si);
	noder.computeNodes(segStrings);
	nodedSegStrings = noder.getNodedSubstrings();
	*numInteriorIntersections = si.numInteriorIntersections;
//System.out.println("# intersection tests: " + si.numTests);
}

void
IteratedNoder::computeNodes(SegmentString::NonConstVect* segStrings)
	// throw(GEOSException);
{
	int numInteriorIntersections;
	nodedSegStrings = segStrings;
	int nodingIterationCount = 0;
	int lastNodesCreated = -1;
	do {
		node(nodedSegStrings, &numInteriorIntersections);
		nodingIterationCount++;
		int nodesCreated = numInteriorIntersections;

		/**
		 * Fail if the number of nodes created is not declining.
		 * However, allow a few iterations at least before doing this
		 */
		//cerr<<"# nodes created: "<<nodesCreated<<endl;
		if (lastNodesCreated > 0
				&& nodesCreated >= lastNodesCreated
				&& nodingIterationCount > maxIter)
		{
			stringstream s;
			s<<"Iterated noding failed to converge after "<<
                                    nodingIterationCount<<" iterations";
			throw util::TopologyException(s.str());
		}
		lastNodesCreated = nodesCreated;

	} while (lastNodesCreated > 0);
	//cerr<<"# nodings = "<<nodingIterationCount<<endl;
}


} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.20  2006/03/15 09:51:12  strk
 * streamlined headers usage
 *
 * Revision 1.19  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.18  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.17  2006/03/02 12:12:00  strk
 * Renamed DEBUG macros to GEOS_DEBUG, all wrapped in #ifndef block to allow global override (bug#43)
 *
 * Revision 1.16  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.15  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.14  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.13  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.12  2005/02/01 13:44:59  strk
 * More profiling labels.
 *
 * Revision 1.11  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.10  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.9  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.7  2004/04/30 09:15:28  strk
 * Enlarged exception specifications to allow for AssertionFailedException.
 * Added missing initializers.
 *
 * Revision 1.6  2004/04/23 00:02:18  strk
 * const-correctness changes
 *
 * Revision 1.5  2004/04/19 16:14:52  strk
 * Some memory leaks plugged in noding algorithms.
 *
 * Revision 1.4  2004/04/19 12:51:01  strk
 * Memory leaks fixes. Throw specifications added.
 *
 * Revision 1.3  2004/04/16 12:48:07  strk
 * Leak fixes.
 *
 * Revision 1.2  2004/04/14 09:30:48  strk
 * Private iterated noding funx now use int* instead of vector to know
 * when it's time to stop.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

