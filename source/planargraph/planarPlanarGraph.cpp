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
 * Revision 1.1  2004/04/07 06:55:50  ybychkov
 * "operation/linemerge" ported from JTS 1.4
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/


#include "../headers/planargraph.h"

namespace geos {
/**
* Constructs a PlanarGraph without any Edges, DirectedEdges, or Nodes.
*/
planarPlanarGraph::planarPlanarGraph(){
	edges=new vector<planarEdge*>();
	dirEdges=new vector<planarDirectedEdge*>();
	nodeMap=new planarNodeMap();
}

planarPlanarGraph::~planarPlanarGraph(){
	delete edges;
	delete dirEdges;
	delete nodeMap;
}
/**
* Returns the Node at the given location, or null if no Node was there.
*/
planarNode* planarPlanarGraph::findNode(Coordinate& pt){
	return nodeMap->find(pt);
}

/**
* Adds a node to the map, replacing any that is already at that location.
* Only subclasses can add Nodes, to ensure Nodes are of the right type.
* @return the added node
*/
void planarPlanarGraph::add(planarNode *node){
	nodeMap->add(node);
}

/**
* Adds the Edge and its DirectedEdges with this PlanarGraph.
* Assumes that the Edge has already been created with its associated DirectEdges.
* Only subclasses can add Edges, to ensure the edges added are of the right class.
*/
void planarPlanarGraph::add(planarEdge *edge){
	edges->push_back(edge);
	add(edge->getDirEdge(0));
	add(edge->getDirEdge(1));
}

/**
* Adds the Edge to this PlanarGraph; only subclasses can add DirectedEdges,
* to ensure the edges added are of the right class.
*/
void planarPlanarGraph::add(planarDirectedEdge *dirEdge){
	dirEdges->push_back(dirEdge);
}
/**
* Returns an Iterator over the Nodes in this PlanarGraph.
*/
map<Coordinate,planarNode*,planarCoordLT>::iterator planarPlanarGraph::nodeIterator()  {
	return nodeMap->iterator();
}

vector<planarNode*>* planarPlanarGraph::getNodes() {
	return nodeMap->getNodes();
}
/**
* Returns an Iterator over the DirectedEdges in this PlanarGraph, in the order in which they
* were added.
*
* @see #add(Edge)
* @see #add(DirectedEdge)
*/
vector<planarDirectedEdge*>::iterator planarPlanarGraph::dirEdgeIterator()  {    
	return dirEdges->begin();  }
/**
* Returns an Iterator over the Edges in this PlanarGraph, in the order in which they
* were added.
*
* @see #add(Edge)
*/
vector<planarEdge*>::iterator planarPlanarGraph::edgeIterator()  {    
	return edges->begin();
}
/**
* Returns the Edges that have been added to this PlanarGraph
* @see #add(Edge)
*/
vector<planarEdge*>* planarPlanarGraph::getEdges()  {
	return edges;
}

/**
* Removes an Edge and its associated DirectedEdges from their from-Nodes and
* from this PlanarGraph. Note: This method does not remove the Nodes associated
* with the Edge, even if the removal of the Edge reduces the degree of a
* Node to zero.
*/
void planarPlanarGraph::remove(planarEdge *edge){
	remove(edge->getDirEdge(0));
	remove(edge->getDirEdge(1));
	for(int i=0;i<(int)edges->size();i++) {
		if((*edges)[i]==edge) {
			edges->erase(edges->begin()+i);
			i--;
		}
	}
}

/**
* Removes DirectedEdge from its from-Node and from this PlanarGraph. Note:
* This method does not remove the Nodes associated with the DirectedEdge,
* even if the removal of the DirectedEdge reduces the degree of a Node to
* zero.
*/
void planarPlanarGraph::remove(planarDirectedEdge *de){
	planarDirectedEdge *sym = de->getSym();
	if (sym!=NULL) sym->setSym(NULL);
	de->getFromNode()->getOutEdges()->remove(de);
	for(int i=0;i<(int)dirEdges->size();i++) {
		if((*dirEdges)[i]==de) {
			dirEdges->erase(dirEdges->begin()+i);
			i--;
		}
	}
}
/**
* Removes a node from the graph, along with any associated DirectedEdges and
* Edges.
*/
void planarPlanarGraph::remove(planarNode *node){
	// unhook all directed edges
	vector<planarDirectedEdge*> *outEdges=node->getOutEdges()->getEdges();
	for(int i=0;i<(int)outEdges->size();i++) {
		planarDirectedEdge *de =(*outEdges)[i];
		planarDirectedEdge *sym = de->getSym();
		// remove the diredge that points to this node
		if (sym!=NULL) remove(sym);
		// remove this diredge from the graph collection
		for(int j=0;j<(int)dirEdges->size();j++) {
			if((*dirEdges)[j]==de) {
				dirEdges->erase(dirEdges->begin()+j);
				j--;
			}
		}
		planarEdge *edge=de->getEdge();
		if (edge!=NULL) {
			for(int k=0;k<(int)edges->size();k++) {
				if((*edges)[k]==edge) {
					edges->erase(edges->begin()+k);
					k--;
				}
			}
		}
	}
	// remove the node from the graph
	nodeMap->remove(node->getCoordinate());
	//nodes.remove(node);
}

/**
* Returns all Nodes with the given number of Edges around it.
*/
vector<planarNode*>* planarPlanarGraph::findNodesOfDegree(int degree){
	vector<planarNode*> *nodesFound=new vector<planarNode*>();
	map<Coordinate,planarNode*,planarCoordLT> *nm=nodeMap->getNodeMap();
	map<Coordinate,planarNode*,planarCoordLT>::iterator	it=nm->begin();
	for (;it!=nm->end();it++) {
		planarNode *node=it->second;
		if (node->getDegree()==degree)
			nodesFound->push_back(node);
	}
	return nodesFound;
}

}
