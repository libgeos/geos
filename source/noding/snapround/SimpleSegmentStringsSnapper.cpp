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
 *
 **********************************************************************/


#include <geos/nodingSnapround.h>

namespace geos {

SimpleSegmentStringsSnapper::SimpleSegmentStringsSnapper() {
	nSnaps = 0;
}

int SimpleSegmentStringsSnapper::getNumSnaps() { 
	return nSnaps;
}


void SimpleSegmentStringsSnapper::computeNodes(vector<SegmentString*>* edges, SegmentSnapper *ss, bool testAllSegments){
	nSnaps = 0;
	for(int i0=0;i0<(int)edges->size();i0++) {
		SegmentString *edge0=(*edges)[i0];
		for(int i1=0;i1<(int)edges->size();i1++) {
			SegmentString *edge1=(*edges)[i1];
			if (testAllSegments || edge0!=edge1)
				computeSnaps(edge0,edge1,ss);
		}
	}
	cout<<"nSnaps = " << nSnaps << endl;
}

/**
* Performs a brute-force comparison of every segment in each SegmentString.
* This has n^2 performance.
*/
void SimpleSegmentStringsSnapper::computeSnaps(SegmentString *e0, SegmentString *e1, SegmentSnapper *ss){
	const CoordinateSequence *pts0=e0->getCoordinates();
	const CoordinateSequence *pts1=e1->getCoordinates();
	for (int i0 = 0; i0 < pts0->getSize()- 1; i0++) {
		for (int i1 = 0; i1 < pts1->getSize()-1; i1++) {
			bool isNodeAdded = ss->addSnappedNode((Coordinate&)pts0->getAt(i0), e1, i1);
			if (isNodeAdded) nSnaps++;
		}
	}
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.4  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.3  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 *
 **********************************************************************/

