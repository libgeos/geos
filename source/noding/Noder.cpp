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

vector<SegmentString*>* Noder::getNodedEdges(vector<SegmentString*>* segStrings) {
	vector<SegmentString*>* resultEdgelist=new vector<SegmentString*>();
	getNodedEdges(segStrings, resultEdgelist);
	return resultEdgelist;
}

void Noder::getNodedEdges(vector<SegmentString*>* segStrings,vector<SegmentString*>* resultEdgelist){
	for (int i=0; i<(int)segStrings->size();i++) {
		SegmentString *ss=(*segStrings)[i];
		ss->getIntersectionList()->addSplitEdges(resultEdgelist);
	}
}

void Noder::setSegmentIntersector(nodingSegmentIntersector *newSegInt){
	segInt=newSegInt;
}
}