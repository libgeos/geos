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
 **********************************************************************/


#include <geos/planargraph.h>

namespace geos {
//namespace planargraph {

/**
* Returns all Edges that connect the two nodes (which are assumed to be different).
*/
vector<planarEdge*>* planarNode::getEdgesBetween(planarNode *node0, planarNode *node1) {
//	vector<planarEdge*> *edges0=planarDirectedEdge::toEdges(node0->getOutEdges()->getEdges());
//Set commonEdges = new HashSet(edges0);
//List edges1 = DirectedEdge.toEdges(node1.getOutEdges().getEdges());
//commonEdges.retainAll(edges1);
//return commonEdges;
	return NULL;
}

/**
* Constructs a Node with the given location.
*/
planarNode::planarNode(const Coordinate& newPt)
{
#ifdef DEBUG_ALLOC
	cerr<<"["<<this<<"] planarNode(const Coordinate&)"<<endl;
#endif
	pt=newPt;
	deStar=new planarDirectedEdgeStar();
}

/*
 * Constructs a Node with the given location and collection of
 * outgoing DirectedEdges.
 */
planarNode::planarNode(Coordinate& newPt, planarDirectedEdgeStar *newDeStar)
{
#ifdef DEBUG_ALLOC
	cerr<<"["<<this<<"] planarNode(const Coordinate&, planarDirectedEdgeStar *)"<<endl;
#endif // DEBUG_ALLOC
	pt=newPt;
	deStar=newDeStar;
}

/**
* Returns the location of this Node.
*/
Coordinate& planarNode::getCoordinate() {
	return pt;
}

/*
 * Adds an outgoing DirectedEdge to this Node.
 */
void
planarNode::addOutEdge(planarDirectedEdge *de)
{
	deStar->add(de);
}

/**
* Returns the collection of DirectedEdges that leave this Node.
*/
planarDirectedEdgeStar*
planarNode::getOutEdges()
{
	return deStar;
}
/**
* Returns the number of edges around this Node.
*/
int planarNode::getDegree() { 
	return deStar->getDegree();
}
/**
* Returns the zero-based index of the given Edge, after sorting in ascending order
* by angle with the positive x-axis.
*/
int planarNode::getIndex(planarEdge *edge){
	return deStar->getIndex(edge);
}

planarNode::~planarNode()
{
#ifdef DEBUG_ALLOC
	cerr<<"["<<this<<"] ~planarNode()"<<endl;
#endif // DEBUG_ALLOC
	delete deStar;
}

//} // namespace planargraph 
} // namespace geos 

/**********************************************************************
 * $Log$
 * Revision 1.4  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.3  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.2  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/04 06:29:11  ybychkov
 * "planargraph" and "geom/utill" upgraded to JTS 1.4
 *
 *
 **********************************************************************/
