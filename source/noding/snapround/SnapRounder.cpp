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
 * Revision 1.1  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 *
 **********************************************************************/


#include "../../headers/nodingSnapround.h"

namespace geos {

void SnapRounder::setLineIntersector(LineIntersector *newLi) {
	li=newLi;
}


vector<SegmentString*>* SnapRounder::node(vector<SegmentString*>* inputSegmentStrings){
	vector<SegmentString*> *resultSegStrings=fullyIntersectSegments(inputSegmentStrings, li);
	NodingValidator *nv=new NodingValidator(resultSegStrings);
	nv->checkValid();
	delete nv;
	return resultSegStrings;
}

vector<SegmentString*>* SnapRounder::fullyIntersectSegments(vector<SegmentString*>* segStrings, LineIntersector *aLi){
	nodingSegmentIntersector *si=NULL;
	vector<SegmentString*> *inputSegStrings=segStrings;
	vector<SegmentString*> *nodedSegStrings=NULL;
	do {
		delete si;
		si=new nodingSegmentIntersector(aLi);
		Noder *noder=new SimpleNoder();
		noder->setSegmentIntersector(si);
		nodedSegStrings=noder->node(inputSegStrings);
		vector<SegmentString*> *snappedSegStrings=computeSnaps(nodedSegStrings);
		cout << "interior ints = " << si->numInteriorIntersections <<endl;
		//System.out.println("snapRounder result");
		//BufferOp.printSegStringList(nodedSegStrings);
		inputSegStrings=snappedSegStrings;
		delete noder;
    } while (si->numInteriorIntersections > 0);
	delete si;
	return nodedSegStrings;
}

/**
* Computes new nodes introduced as a result of snapping segments to near vertices
* @param li
*/
vector<SegmentString*>* SnapRounder::computeSnaps(vector<SegmentString*> *segStrings){
	vector<SegmentString*> *splitSegStringList=NULL;
	int numSnaps;
	/**
	* Have to snap repeatedly, because snapping a line may move it enough
	* that it crosses another hot pixel.
	*/
	do {
		SimpleSegmentStringsSnapper *snapper = new SimpleSegmentStringsSnapper();
		SegmentSnapper *ss = new SegmentSnapper();
		snapper->computeNodes(segStrings, ss, true);
		numSnaps = snapper->getNumSnaps();
		// save the list of split seg strings in case we are going to return it
		splitSegStringList = Noder::getNodedEdges(segStrings);
		segStrings = splitSegStringList;
	} while (numSnaps > 0);
	return splitSegStringList;
}

}
