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
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../headers/planargraph.h"

namespace geos {

/**
* Constructs an Edge whose DirectedEdges are not yet set. Be sure to call
* {@link #setDirectedEdges(DirectedEdge, DirectedEdge)}
*/
planarEdge::planarEdge() {}

/**
* Constructs an Edge initialized with the given DirectedEdges, and for each
* DirectedEdge: sets the Edge, sets the symmetric DirectedEdge, and adds
* this Edge to its from-Node.
*/
planarEdge::planarEdge(planarDirectedEdge *de0, planarDirectedEdge *de1){
	setDirectedEdges(de0, de1);
}

/**
* Initializes this Edge's two DirectedEdges, and for each DirectedEdge: sets the
* Edge, sets the symmetric DirectedEdge, and adds this Edge to its from-Node.
*/
void planarEdge::setDirectedEdges(planarDirectedEdge *de0, planarDirectedEdge *de1){
	dirEdge=new vector<planarDirectedEdge*>();
	dirEdge->push_back(de0);
	dirEdge->push_back(de1);
	de0->setEdge(this);
	de1->setEdge(this);
	de0->setSym(de1);
	de1->setSym(de0);
	de0->getFromNode()->addOutEdge(de0);
	de1->getFromNode()->addOutEdge(de1);
}

/**
* Returns one of the DirectedEdges associated with this Edge.
* @param i 0 or 1
*/
planarDirectedEdge* planarEdge::getDirEdge(int i){
	return (*dirEdge)[i];
}

/**
* Returns the {@link DirectedEdge} that starts from the given node, or null if the
* node is not one of the two nodes associated with this Edge.
*/
planarDirectedEdge* planarEdge::getDirEdge(planarNode *fromNode){
	if ((*dirEdge)[0]->getFromNode()==fromNode) return (*dirEdge)[0];
	if ((*dirEdge)[1]->getFromNode()==fromNode) return (*dirEdge)[1];
	// node not found
	// possibly should throw an exception here?
	return NULL;
}

/**
* If <code>node</code> is one of the two nodes associated with this Edge,
* returns the other node; otherwise returns null.
*/
planarNode* planarEdge::getOppositeNode(planarNode *node){
	if ((*dirEdge)[0]->getFromNode()==node) return (*dirEdge)[0]->getToNode();
	if ((*dirEdge)[1]->getFromNode()==node) return (*dirEdge)[1]->getToNode();
	// node not found
	// possibly should throw an exception here?
	return NULL;
}
}
