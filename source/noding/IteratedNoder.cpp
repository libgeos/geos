/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding.h>
#include <geos/profiler.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {

#if PROFILE
static Profiler *profiler = Profiler::instance();
#endif


IteratedNoder::IteratedNoder(const PrecisionModel *newPm)
{
	//li = new LineIntersector();
	pm=newPm;
	li.setPrecisionModel(pm);
}

IteratedNoder::~IteratedNoder() {
	//delete li;
}

/*
 * Fully nodes a list of SegmentStrings, i.e. peforms noding iteratively
 * until no intersections are found between segments.
 * Maintains labelling of edges correctly through
 * the noding.
 *
 * @param segStrings a collection of SegmentStrings to be noded
 * @return a collection of the noded SegmentStrings
 * @throws TopologyException if the iterated noding fails to converge.
 */
vector<SegmentString*>*
IteratedNoder::node(vector<SegmentString*> *segStrings)
	// throw(GEOSException *)
{
	int numInteriorIntersections;

	vector<SegmentString*> *nodedEdges=new vector<SegmentString *>(*segStrings);

	int nodingIterationCount = 0;
	int lastNodesCreated = -1;
	do {
		vector<SegmentString*> *oString = nodedEdges;
		nodedEdges=node(nodedEdges,&numInteriorIntersections);
		delete oString;
		nodingIterationCount++;
		int nodesCreated=numInteriorIntersections;
		//System.out.println("# nodes created: " + nodesCreated);
		if (lastNodesCreated > 0 && nodesCreated > lastNodesCreated) {
			delete nodedEdges;
			throw new TopologyException("Iterated noding failed to converge");
		}
		lastNodesCreated = nodesCreated;
		//saveEdges(nodedEdges, "run" + runCount + "_nodedEdges");
	} while (lastNodesCreated > 0);
	//System.out.println("# nodings = " + nodingIterationCount);
#if DEBUG
	cerr<<"IteratedNoder::node iterated "<<nodingIterationCount<<" times"<<endl;
#endif
	return nodedEdges;
}


/*
 * Node the input segment strings once
 * and create the split edges between the nodes
 */
vector<SegmentString*>*
IteratedNoder::node(vector<SegmentString*> *segStrings, int *numInteriorIntersections)
{
	//nodingSegmentIntersector *si = new nodingSegmentIntersector(li);
	nodingSegmentIntersector si(&li);
	MCQuadtreeNoder noder;
	noder.setSegmentIntersector(&si);
	// perform the noding
#if PROFILE
	static Profile *prof = profiler->get("IteratedNoder::node");
	prof->start();
#endif
	vector<SegmentString*> *nodedSegStrings=noder.node(segStrings);
#if PROFILE
	prof->stop();
#endif
	*numInteriorIntersections=si.numInteriorIntersections;
	//System.out.println("# intersection tests: " + si.numTests);

	//delete noder;
	//delete si;
	return nodedSegStrings;
}

}

/**********************************************************************
 * $Log$
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

