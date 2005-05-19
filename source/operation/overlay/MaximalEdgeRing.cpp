/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/opOverlay.h>
#include <stdio.h>

namespace geos {

// CGAlgorithms obsoleted
MaximalEdgeRing::MaximalEdgeRing(DirectedEdge *start,const GeometryFactory *geometryFactory, CGAlgorithms *cga):
	EdgeRing(start,geometryFactory)
{
	computePoints(start);
	computeRing();
}

MaximalEdgeRing::~MaximalEdgeRing(){
}

DirectedEdge* MaximalEdgeRing::getNext(DirectedEdge *de) {
	return de->getNext();
}

void MaximalEdgeRing::setEdgeRing(DirectedEdge *de,EdgeRing *er) {
	de->setEdgeRing(er);
}

/**
* For all nodes in this EdgeRing,
* link the DirectedEdges at the node to form minimalEdgeRings
*/
void MaximalEdgeRing::linkDirectedEdgesForMinimalEdgeRings() {
	DirectedEdge* de=startDe;
	do {
		Node* node=de->getNode();
		((DirectedEdgeStar*) node->getEdges())->linkMinimalDirectedEdges(this);
		de=de->getNext();
	} while (de!=startDe);
}

// This function returns a newly allocated vector of
// pointers to newly allocated MinimalEdgeRing objects.
//
vector<MinimalEdgeRing*>* MaximalEdgeRing::buildMinimalRings() {
	vector<MinimalEdgeRing*> *minEdgeRings=new vector<MinimalEdgeRing*>;
	DirectedEdge *de=startDe;
	do {
		if(de->getMinEdgeRing()==NULL) {
			MinimalEdgeRing *minEr=new MinimalEdgeRing(de,geometryFactory);
			minEdgeRings->push_back(minEr);
		}
		de=de->getNext();
	} while(de!=startDe);
	return minEdgeRings;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.12  2005/05/19 10:29:28  strk
 * Removed some CGAlgorithms instances substituting them with direct calls
 * to the static functions. Interfaces accepting CGAlgorithms pointers kept
 * for backward compatibility but modified to make the argument optional.
 * Fixed a small memory leak in OffsetCurveBuilder::getRingCurve.
 * Inlined some smaller functions encountered during bug hunting.
 * Updated Copyright notices in the touched files.
 *
 * Revision 1.11  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/06/30 20:59:13  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.8  2003/10/13 17:47:49  strk
 * delete statement removed
 *
 **********************************************************************/

