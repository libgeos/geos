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
 **********************************************************************
 * $Log$
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


#include "../headers/noding.h"

namespace geos {

IteratedNoder::IteratedNoder(const PrecisionModel *newPm) {
	li = new RobustLineIntersector();
	pm=newPm;
    li->setPrecisionModel(pm);
}

IteratedNoder::~IteratedNoder() {
	delete li;
}
/**
* Fully nodes a list of {@link SegmentStrings}, i.e. peforms noding iteratively
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
	return nodedEdges;
}


/**
* Node the input segment strings once
* and create the split edges between the nodes
*/
vector<SegmentString*>*
IteratedNoder::node(vector<SegmentString*> *segStrings, int *numInteriorIntersections)
{
	nodingSegmentIntersector *si=new nodingSegmentIntersector(li);
	MCQuadtreeNoder *noder = new MCQuadtreeNoder();
	noder->setSegmentIntersector(si);
	// perform the noding
	vector<SegmentString*> *nodedSegStrings=noder->node(segStrings);
	*numInteriorIntersections=si->numInteriorIntersections;
	//System.out.println("# intersection tests: " + si.numTests);

	delete noder;
	delete si;
	return nodedSegStrings;
}

}
