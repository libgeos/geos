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
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/14 06:04:26  ybychkov
 * "geomgraph/index" committ problem fixed.
 *
 * Revision 1.6  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.4  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/


#include <geos/geomgraphindex.h>

namespace geos {

SimpleEdgeSetIntersector::SimpleEdgeSetIntersector() {
	nOverlaps=0;
}

void SimpleEdgeSetIntersector::computeIntersections(vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments){
	nOverlaps = 0;
	for(int i0=0;i0<(int)edges->size();i0++) {
		Edge *edge0=(*edges)[i0];
		for(int i1=0;i1<(int)edges->size();i1++) {
			Edge *edge1=(*edges)[i1];
			if (testAllSegments || edge0!=edge1)
				computeIntersects(edge0,edge1,si);
		}
	}
}


void SimpleEdgeSetIntersector::computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si){
	nOverlaps = 0;
	for(int i0=0;i0<(int)edges0->size();i0++) {
		Edge *edge0=(*edges0)[i0];
		for(int i1=0;i1<(int)edges1->size();i1++) {
			Edge *edge1=(*edges1)[i1];
			computeIntersects(edge0,edge1,si);
		}
	}
}

/**
* Performs a brute-force comparison of every segment in each Edge.
* This has n^2 performance, and is about 100 times slower than using
* monotone chains.
*/
void SimpleEdgeSetIntersector::computeIntersects(Edge *e0,Edge *e1,SegmentIntersector *si){
	const CoordinateList *pts0=e0->getCoordinates();
	const CoordinateList *pts1=e1->getCoordinates();
	for(int i0=0;i0<pts0->getSize()-1;i0++) {
		for(int i1=0;i1<pts1->getSize()-1;i1++) {
			si->addIntersections(e0,i0,e1,i1);
		}
	}
}
}

