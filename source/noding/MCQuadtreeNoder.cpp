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
 * Revision 1.5  2004/05/05 15:51:29  strk
 * Fixed big leak in intersectChains()
 *
 * Revision 1.4  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.3  2004/04/19 16:14:52  strk
 * Some memory leaks plugged in noding algorithms.
 *
 * Revision 1.2  2004/04/16 12:48:07  strk
 * Leak fixes.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/


#include "../headers/noding.h"

namespace geos {

MCQuadtreeNoder::MCQuadtreeNoder(){
	chains=new vector<indexMonotoneChain*>();
	index=new STRtree();
	idCounter = 0;
	nOverlaps = 0;
}

MCQuadtreeNoder::~MCQuadtreeNoder(){
	for (int i=0; i<chains->size(); i++)
	{
		delete (*chains)[i];
	}
	delete chains;
	delete index;
}
vector<SegmentString*> *
MCQuadtreeNoder::node(vector<SegmentString*> *inputSegStrings)
{
	for(int i=0; i<(int)inputSegStrings->size();i++) {
		add((*inputSegStrings)[i]);
	}
	intersectChains();
	//System.out.println("MCQuadtreeNoder: # chain overlaps = " + nOverlaps);
	vector<SegmentString*> *nodedSegStrings=getNodedEdges(inputSegStrings);
	return nodedSegStrings;
}

void MCQuadtreeNoder::intersectChains() {
	MonotoneChainOverlapAction *overlapAction = new SegmentOverlapAction(segInt);
	for (int i=0; i<(int)chains->size();i++) {
		indexMonotoneChain *queryChain=(*chains)[i];
		vector<void*> *overlapChains =index->query(queryChain->getEnvelope());
		for (int j=0; j<(int)overlapChains->size();j++) {
			indexMonotoneChain *testChain=(indexMonotoneChain*)(*overlapChains)[j];
			/**
			* following test makes sure we only compare each pair of chains once
			* and that we don't compare a chain to itself
			*/
			if (testChain->getId()>queryChain->getId()) {
				queryChain->computeOverlaps(testChain, overlapAction);
				nOverlaps++;
			}
		}
		delete overlapChains;
	}

	delete overlapAction;
}

void MCQuadtreeNoder::add(SegmentString *segStr) {
	vector<indexMonotoneChain*> *segChains=MonotoneChainBuilder::getChains((CoordinateList*)segStr->getCoordinates(),segStr);
	for (int i=0; i<(int)segChains->size();i++) {
		indexMonotoneChain *mc=(*segChains)[i];
		mc->setId(idCounter++);
		index->insert(mc->getEnvelope(), mc);
		chains->push_back(mc);
	}
	delete segChains;
}

MCQuadtreeNoder::SegmentOverlapAction::SegmentOverlapAction(nodingSegmentIntersector *newSi){
	si=newSi;
}

void MCQuadtreeNoder::SegmentOverlapAction::overlap(indexMonotoneChain *mc1, int start1, indexMonotoneChain *mc2, int start2) {
	SegmentString *ss1=(SegmentString*) mc1->getContext();
	SegmentString *ss2=(SegmentString*) mc2->getContext();
	si->processIntersections(ss1, start1, ss2, start2);
}

}

