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
 *
 **********************************************************************/


#include <geos/opLinemerge.h>

namespace geos {

/*
 * Adds an Edge, DirectedEdges, and Nodes for the given LineString
 * representation of an edge. 
 */
void
LineMergeGraph::addEdge(LineString *lineString)
{
	if (lineString->isEmpty())
	{
		return;
	}

	CoordinateSequence *coordinates=CoordinateSequence::removeRepeatedPoints(lineString->getCoordinatesRO());
	const Coordinate& startCoordinate=coordinates->getAt(0);
	const Coordinate& endCoordinate=coordinates->getAt(coordinates->getSize()-1);
	planarNode* startNode=getNode(startCoordinate);
	planarNode* endNode=getNode(endCoordinate);
	planarDirectedEdge *directedEdge0=new LineMergeDirectedEdge(startNode, endNode,coordinates->getAt(1), true);
	planarDirectedEdge *directedEdge1=new LineMergeDirectedEdge(endNode, startNode,coordinates->getAt(coordinates->getSize()-2),false);
	newDirEdges.push_back(directedEdge0);
	newDirEdges.push_back(directedEdge1);
	planarEdge *edge=new LineMergeEdge(lineString);
	newEdges.push_back(edge);
	edge->setDirectedEdges(directedEdge0, directedEdge1);
	add(edge);
	delete coordinates;
}

planarNode *
LineMergeGraph::getNode(const Coordinate &coordinate)
{
	planarNode *node=findNode(coordinate);
	if (node==NULL) {
		node=new planarNode(coordinate);
		newNodes.push_back(node);
		add(node);
	}
	return node;
}

LineMergeGraph::~LineMergeGraph()
{
	for (int i=0; i<newNodes.size(); i++) delete newNodes[i];
	for (int i=0; i<newEdges.size(); i++) delete newEdges[i];
	for (int i=0; i<newDirEdges.size(); i++) delete newDirEdges[i];
}

}

/**********************************************************************
 * $Log$
 * Revision 1.4  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.3  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:28  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 **********************************************************************/
