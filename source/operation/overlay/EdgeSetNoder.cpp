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
 * $Log$
 * Revision 1.6  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/opOverlay.h>
#include <stdio.h>

namespace geos {

EdgeSetNoder::EdgeSetNoder(LineIntersector *newLi) {
	li=newLi;
	inputEdges=new vector<Edge*>();
}

EdgeSetNoder::~EdgeSetNoder() {
	delete inputEdges;
}

void EdgeSetNoder::addEdges(vector<Edge*> *edges){
	inputEdges->insert(inputEdges->end(),edges->begin(),edges->end());
}

vector<Edge*>* EdgeSetNoder::getNodedEdges() {
	EdgeSetIntersector *esi=new SimpleMCSweepLineIntersector();
	SegmentIntersector *si=new SegmentIntersector(li,true,false);
	esi->computeIntersections(inputEdges,si,true);
	//Debug.println("has proper int = " + si.hasProperIntersection());
	vector<Edge*> *splitEdges=new vector<Edge*>();
	for(int i=0;i<(int)inputEdges->size();i++) {
		Edge* e=(*inputEdges)[i];
		e->getEdgeIntersectionList()->addSplitEdges(splitEdges);
	}
	return splitEdges;
}
}

