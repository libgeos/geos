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
 * Revision 1.10  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/opRelate.h>
#include <stdio.h>

namespace geos {

RelateNodeGraph::RelateNodeGraph() {
	nodes=new NodeMap(new RelateNodeFactory());
}

RelateNodeGraph::~RelateNodeGraph() {
	delete nodes;
}

map<Coordinate,Node*,CoordLT>* RelateNodeGraph::getNodeMap() {
	return nodes->nodeMap;
}

void RelateNodeGraph::build(GeometryGraph *geomGraph) {
	// compute nodes for intersections between previously noded edges
	computeIntersectionNodes(geomGraph,0);
	/**
	* Copy the labelling for the nodes in the parent Geometry.  These override
	* any labels determined by intersections.
	*/
	copyNodesAndLabels(geomGraph,0);
	/**
	* Build EdgeEnds for all intersections.
	*/
	EdgeEndBuilder *eeBuilder=new EdgeEndBuilder();
	vector<EdgeEnd*> *eeList=eeBuilder->computeEdgeEnds(geomGraph->getEdges());
	insertEdgeEnds(eeList);
	delete eeBuilder;
	delete eeList;
	//Debug.println("==== NodeList ===");
	//Debug.print(nodes);
}

/**
* Insert nodes for all intersections on the edges of a Geometry.
* Label the created nodes the same as the edge label if they do not already have a label.
* This allows nodes created by either self-intersections or
* mutual intersections to be labelled.
* Endpoint nodes will already be labelled from when they were inserted.
* <p>
* Precondition: edge intersections have been computed.
*/
void RelateNodeGraph::computeIntersectionNodes(GeometryGraph *geomGraph, int argIndex) {
	vector<Edge*> *edges=geomGraph->getEdges();
	for(vector<Edge*>::iterator edgeIt=edges->begin();edgeIt<edges->end();edgeIt++) {
		Edge *e=*edgeIt;
		int eLoc=e->getLabel()->getLocation(argIndex);
		vector<EdgeIntersection*> *eiL=e->getEdgeIntersectionList()->list;
		for(vector<EdgeIntersection*>::iterator eiIt=eiL->begin();eiIt<eiL->end();eiIt++) {
			EdgeIntersection *ei=*eiIt;
			RelateNode *n=(RelateNode*) nodes->addNode(ei->coord);
			if (eLoc==Location::BOUNDARY)
				n->setLabelBoundary(argIndex);
			else {
				if (n->getLabel()->isNull(argIndex))
					n->setLabel(argIndex,Location::INTERIOR);
			}
			//Debug.println(n);
		}
//		delete eiL;
	}
}
/**
* Copy all nodes from an arg geometry into this graph.
* The node label in the arg geometry overrides any previously computed
* label for that argIndex.
* (E.g. a node may be an intersection node with
* a computed label of BOUNDARY,
* but in the original arg Geometry it is actually
* in the interior due to the Boundary Determination Rule)
*/
void RelateNodeGraph::copyNodesAndLabels(GeometryGraph *geomGraph,int argIndex) {
	map<Coordinate,Node*,CoordLT> *nMap=geomGraph->getNodeMap()->nodeMap;
	map<Coordinate,Node*,CoordLT>::iterator nodeIt;
	for(nodeIt=nMap->begin();nodeIt!=nMap->end();nodeIt++) {
		Node *graphNode=nodeIt->second;
		Node *newNode=nodes->addNode(graphNode->getCoordinate());
		newNode->setLabel(argIndex,graphNode->getLabel()->getLocation(argIndex));
		//node.print(System.out);
	}
}

void RelateNodeGraph::insertEdgeEnds(vector<EdgeEnd*> *ee){
	for(vector<EdgeEnd*>::iterator i=ee->begin();i<ee->end();i++) {
		EdgeEnd *e=*i;
		nodes->add(e);
	}
}
}

