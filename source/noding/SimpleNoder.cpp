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
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/


#include <geos/noding.h>

namespace geos {

vector<SegmentString*>* SimpleNoder::node(vector<SegmentString*>* inputSegStrings) {
	for (int i0=0;i0<(int)inputSegStrings->size();i0++) {
		SegmentString *edge0=(*inputSegStrings)[i0];
		for (int i1=0;i1<(int)inputSegStrings->size();i1++) {
			SegmentString *edge1=(*inputSegStrings)[i1];
			computeIntersects(edge0, edge1);
		}
	}
	vector<SegmentString*> *nodedSegStrings=getNodedEdges(inputSegStrings);
	return nodedSegStrings;
}

void SimpleNoder::computeIntersects(SegmentString *e0, SegmentString *e1){
	const CoordinateList *pts0=e0->getCoordinates();
	const CoordinateList *pts1=e1->getCoordinates();
	for (int i0=0;i0<pts0->getSize()-1;i0++) {
		for (int i1=0;i1<pts1->getSize()-1;i1++) {
			segInt->processIntersections(e0, i0, e1, i1);
		}
	}
}

}
