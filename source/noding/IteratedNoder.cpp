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
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/


#include "../headers/noding.h"

namespace geos {

IteratedNoder::IteratedNoder(PrecisionModel *newPm) {
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
vector<SegmentString*>* IteratedNoder::node(vector<SegmentString*> *segStrings) {
	vector<int> *numInteriorIntersections=new vector<int>();
	vector<SegmentString*> *nodedEdges=segStrings;
	int nodingIterationCount = 0;
	int lastNodesCreated = -1;
	do {
		nodedEdges=node(nodedEdges,numInteriorIntersections);
		nodingIterationCount++;
		int nodesCreated=(*numInteriorIntersections)[0];
		//System.out.println("# nodes created: " + nodesCreated);
		if (lastNodesCreated > 0 && nodesCreated > lastNodesCreated) {
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
vector<SegmentString*>* IteratedNoder::node(vector<SegmentString*> *segStrings,vector<int> *numInteriorIntersections){
	nodingSegmentIntersector *si=new nodingSegmentIntersector(li);
	MCQuadtreeNoder *noder = new MCQuadtreeNoder();
	noder->setSegmentIntersector(si);
	// perform the noding
	vector<SegmentString*> *nodedSegStrings=noder->node(segStrings);
	(*numInteriorIntersections)[0]=si->numInteriorIntersections;
	//System.out.println("# intersection tests: " + si.numTests);
	return nodedSegStrings;
}

}
