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
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/


#include "../../headers/opLinemerge.h"

namespace geos {
/**
* Constructs an EdgeString with the given factory used to convert this EdgeString
* to a LineString
*/
EdgeString::EdgeString(GeometryFactory *newFactory) {
	directedEdges=new vector<LineMergeDirectedEdge*>();
	coordinates=NULL;
	factory=newFactory;
}

EdgeString::~EdgeString() {
	delete directedEdges;
}

/**
* Adds a directed edge which is known to form part of this line.
*/
void EdgeString::add(LineMergeDirectedEdge *directedEdge) {
	directedEdges->push_back(directedEdge);
}

const CoordinateList* EdgeString::getCoordinates() {
	if (coordinates==NULL) {
		int forwardDirectedEdges = 0;
		int reverseDirectedEdges = 0;
		coordinates=CoordinateListFactory::internalFactory->createCoordinateList();
		for (int i=0;i<(int)directedEdges->size();i++) {
			LineMergeDirectedEdge* directedEdge=(*directedEdges)[i];
			if (directedEdge->getEdgeDirection()) {
				forwardDirectedEdges++;
			} else {
				reverseDirectedEdges++;
			}
			coordinates->add(((LineMergeEdge*)directedEdge->getEdge())->getLine()->getCoordinates(),false,directedEdge->getEdgeDirection());
		}
		if (reverseDirectedEdges > forwardDirectedEdges) {
			CoordinateList::reverse(coordinates);
		}
	}
	return coordinates;
}

/**
* Converts this EdgeString into a LineString.
*/
LineString* EdgeString::toLineString() {
	return factory->createLineString(getCoordinates());
}
}
