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
 * Revision 1.2  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 *
 **********************************************************************/


#include <geos/opLinemerge.h>

namespace geos {
/**
* Adds an Edge, DirectedEdges, and Nodes for the given LineString representation
* of an edge. 
*/
void LineMergeGraph::addEdge(LineString *lineString) {
	if (lineString->isEmpty()) {
		return;
	}

	CoordinateList *coordinates=CoordinateList::removeRepeatedPoints(lineString->getCoordinates());
	Coordinate& startCoordinate=(Coordinate)coordinates->getAt(0);
	Coordinate& endCoordinate=(Coordinate)coordinates->getAt(coordinates->getSize()-1);
	planarNode* startNode=getNode(startCoordinate);
	planarNode* endNode=getNode(endCoordinate);
	planarDirectedEdge *directedEdge0=new LineMergeDirectedEdge(startNode, endNode,(Coordinate)coordinates->getAt(1), true);
	planarDirectedEdge *directedEdge1=new LineMergeDirectedEdge(endNode, startNode,(Coordinate)coordinates->getAt(coordinates->getSize()-2),false);
	planarEdge *edge=new LineMergeEdge(lineString);
	edge->setDirectedEdges(directedEdge0, directedEdge1);
	add(edge);
}

planarNode* LineMergeGraph::getNode(Coordinate &coordinate) {
	planarNode *node=findNode(coordinate);
	if (node==NULL) {
		node=new planarNode(coordinate);
		add(node);
	}
	return node;
}
}