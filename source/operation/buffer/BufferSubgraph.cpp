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
 * Revision 1.4  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include "../../headers/opBuffer.h"

namespace geos {

BufferSubgraph::BufferSubgraph(CGAlgorithms *newCga) {
	finder=new RightmostEdgeFinder(newCga);
	dirEdgeList=new vector<DirectedEdge*>();
	nodes=new vector<Node*>();
	rightMostCoord.setNull();
}

BufferSubgraph::~BufferSubgraph() {
	delete finder;
	delete dirEdgeList;
	delete nodes;
}

vector<DirectedEdge*>* BufferSubgraph::getDirectedEdges() {
	return dirEdgeList;
}

vector<Node*>* BufferSubgraph::getNodes() {
	return nodes;
}
/**
*Get the rightMost coordinate in the edges of the subgraph
*/
Coordinate& BufferSubgraph::getRightmostCoordinate() {
	return rightMostCoord;
}

void BufferSubgraph::create(Node *node){
	add(node);
	finder->findEdge(dirEdgeList);
	rightMostCoord=finder->getCoordinate();
}

void BufferSubgraph::add(Node *node) {
	node->setVisited(true);
	nodes->push_back(node);
	vector<EdgeEnd*> *ee=node->getEdges()->getEdges();
	for(int i=0; i<(int)ee->size();i++) {
		DirectedEdge *de=(DirectedEdge*)(*ee)[i];
		dirEdgeList->push_back(de);
		DirectedEdge *sym=de->getSym();
		Node *symNode=sym->getNode();
		/**
		*NOTE: this is a depth-first traversal of the graph.
		*This will cause a large depth of recursion.
		*It might be better to do a breadth-first traversal.
		*/
		if (!symNode->isVisited()) add(symNode);
	}
}

void BufferSubgraph::clearVisitedEdges() {
	for(int i=0; i<(int)dirEdgeList->size();i++) {
		DirectedEdge *de=(*dirEdgeList)[i];
		de->setVisited(false);
	}
}

void BufferSubgraph::computeDepth(int outsideDepth) {
	clearVisitedEdges();
	// find an outside edge to assign depth to
	DirectedEdge *de=finder->getEdge();
	Node *n=de->getNode();
	Label *label=de->getLabel();
	// right side of line returned by finder is on the outside
	de->setEdgeDepths(Position::RIGHT,outsideDepth);
	computeNodeDepth(n,de);
}

void BufferSubgraph::computeNodeDepth(Node *n,DirectedEdge *startEdge) {
	if (startEdge->isVisited()) return;
	((DirectedEdgeStar*)n->getEdges())->computeDepths(startEdge);
	// copy depths to sym edges
	vector<EdgeEnd*> *ee=n->getEdges()->getEdges();
	for(int i=0; i<(int)ee->size();i++) {
		DirectedEdge *de=(DirectedEdge*)(*ee)[i];
		de->setVisited(true);
		DirectedEdge *sym=de->getSym();
		sym->setDepth(Position::LEFT,de->getDepth(Position::RIGHT));
		sym->setDepth(Position::RIGHT,de->getDepth(Position::LEFT));
	}
	// propagate depth to all linked nodes via the sym edges
	// If a sym edge has been visited already, there is no need to process it further
	ee=n->getEdges()->getEdges();
	for(int i=0; i<(int)ee->size();i++) {
		DirectedEdge *de=(DirectedEdge*)(*ee)[i];
		DirectedEdge *sym=de->getSym();
		Node *symNode=sym->getNode();
		/**
		*NOTE: this is a depth-first traversal of the graph.
		*This will cause a large depth of recursion.
		*It might be better to do a breadth-first traversal.
		*/
		computeNodeDepth(symNode, sym);
	}
}

/**
*Find all edges whose depths indicates that they are in the result area(s).
*Since we want polygon shells to be
*oriented CW, choose dirEdges with the interior of the result on the RHS.
*Mark them as being in the result.
*Interior Area edges are the result of dimensional collapses.
*They do not form part of the result area boundary.
*/
void BufferSubgraph::findResultEdges() {
	for(int i=0; i<(int)dirEdgeList->size();i++) {
		DirectedEdge *de=(*dirEdgeList)[i];
		/**
		*Select edges which have the EXTERIOR on the L and INTERIOR
		*on the right.  It doesn't matter how deep the interior is.
		*/
		if (de->getDepth(Position::RIGHT)>=1
			&& de->getDepth(Position::LEFT)==0
			&& !de->isInteriorAreaEdge()) {
				de->setInResult(true);
				//Debug.print("in result "); Debug.println(de);
		}
	}
}

/**
*BufferSubgraphs are compared on the x-value of their rightmost Coordinate.
*This defines a partial ordering on the graphs such that:
*<p>
*g1>=g2 <==> Ring(g2) does not contain Ring(g1)
*<p>
*where Polygon(g) is the buffer polygon that is built from g.
*<p>
*This relationship is used to sort the BufferSubgraphs so that shells are guaranteed to
*be built before holes.
*/
int BufferSubgraph::compareTo(void* o) {
	BufferSubgraph *bsgraph=(BufferSubgraph*) o;
	return compareTo(bsgraph);
}

int BufferSubgraph::compareTo(BufferSubgraph *bsg) {
	if (rightMostCoord.x<bsg->rightMostCoord.x) {
		return -1;
	}
	if (rightMostCoord.x>bsg->rightMostCoord.x) {
		return 1;
	}
	return 0;
}
}

