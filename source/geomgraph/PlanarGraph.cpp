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
 * Revision 1.4  2004/05/03 10:43:42  strk
 * Exception specification considered harmful - left as comment.
 *
 * Revision 1.3  2004/04/19 15:14:45  strk
 * Added missing virtual destructor in SpatialIndex class.
 * Memory leaks fixes. Const and throw specifications added.
 *
 * Revision 1.2  2004/04/10 08:40:01  ybychkov
 * "operation/buffer" upgraded to JTS 1.4
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.19  2003/11/12 18:02:56  strk
 * Added throw specification. Fixed leaks on exceptions.
 *
 * Revision 1.18  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.17  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/


#include "../headers/geomgraph.h"

namespace geos {

CGAlgorithms* PlanarGraph::cga=new CGAlgorithms();
//LineIntersector* PlanarGraph::li=new RobustLineIntersector();

 /**
 * For nodes in the vector, link the DirectedEdges at the node that are in the result.
 * This allows clients to link only a subset of nodes in the graph, for
 * efficiency (because they know that only a subset is of interest).
 */
void
PlanarGraph::linkResultDirectedEdges(vector<Node*> *allNodes)
	//throw(TopologyException *)
{
	vector<Node*>::iterator nodeit;
	for(nodeit=allNodes->begin(); nodeit<allNodes->end(); nodeit++)
	{
		Node *node=*nodeit;
		// this might throw an exception
		((DirectedEdgeStar*) node->getEdges())->linkResultDirectedEdges();
	}
}

PlanarGraph::PlanarGraph(NodeFactory *nodeFact) {
	nodes=new NodeMap(nodeFact);
	edges=new vector<Edge*>();
	edgeEndList=new vector<EdgeEnd*>();

}

PlanarGraph::PlanarGraph(){
	nodes=new NodeMap(new NodeFactory());
	edges=new vector<Edge*>();
	edgeEndList=new vector<EdgeEnd*>();
}

PlanarGraph::~PlanarGraph(){
	delete nodes;
	int i;
	for(i=0;i<(int)edges->size();i++) {
		delete (*edges)[i];
	}
	delete edges;
	for(i=0;i<(int)edgeEndList->size();i++) {
		delete (*edgeEndList)[i];
	}
	delete edgeEndList;
}

vector<Edge*>::iterator PlanarGraph::getEdgeIterator() {
	return edges->begin();
}
vector<EdgeEnd*> *PlanarGraph::getEdgeEnds(){
	return edgeEndList;
}

bool PlanarGraph::isBoundaryNode(int geomIndex,Coordinate& coord){
	Node *node=nodes->find(coord);
	if (node==NULL) return false;
	Label *label=node->getLabel();
	if (label!=NULL && label->getLocation(geomIndex)==Location::BOUNDARY) return true;
	return false;
}

void PlanarGraph::insertEdge(Edge *e){
	edges->push_back(e);
}

void PlanarGraph::add(EdgeEnd *e){
	nodes->add(e);
	edgeEndList->push_back(e);
}

map<Coordinate,Node*,CoordLT>::iterator PlanarGraph::getNodeIterator() {
	return nodes->iterator();
}

vector<Node*>* PlanarGraph::getNodes(){
	vector<Node*> *values=new vector<Node*>();
	map<Coordinate,Node*,CoordLT>::iterator it=nodes->nodeMap->begin();
	while(it!=nodes->nodeMap->end()) {
		values->push_back(it->second);
		it++;
	}
	return values;
}

// arg cannot be const, NodeMap::addNode will
// occasionally label-merge first arg.
Node* PlanarGraph::addNode(Node *node) {
	return nodes->addNode(node);
}

Node* PlanarGraph::addNode(const Coordinate& coord) {
	return nodes->addNode(coord);
}

/**
* @return the node if found; null otherwise
*/
Node* PlanarGraph::find(Coordinate& coord) {
	return nodes->find(coord);
}

/**
* Add a set of edges to the graph.  For each edge two DirectedEdges
* will be created.  DirectedEdges are NOT linked by this method.
*/
void PlanarGraph::addEdges(vector<Edge*>* edgesToAdd){
	// create all the nodes for the edges
	for (vector<Edge*>::iterator it=edgesToAdd->begin();it<edgesToAdd->end();it++) {
		Edge *e=*it;
		edges->push_back(e);
		DirectedEdge *de1=new DirectedEdge(e, true);
		DirectedEdge *de2=new DirectedEdge(e, false);
		de1->setSym(de2);
		de2->setSym(de1);
		add(de1);
		add(de2);
	}
}

/**
* Link the DirectedEdges at the nodes of the graph.
* This allows clients to link only a subset of nodes in the graph, for
* efficiency (because they know that only a subset is of interest).
*/
void PlanarGraph::linkResultDirectedEdges(){
	for (map<Coordinate,Node*,CoordLT>::iterator nodeit=nodes->iterator();nodeit!=nodes->nodeMap->end();nodeit++) {
		Node *node=nodeit->second;
		((DirectedEdgeStar*)node->getEdges())->linkResultDirectedEdges();
	}
}

/**
* Link the DirectedEdges at the nodes of the graph.
* This allows clients to link only a subset of nodes in the graph, for
* efficiency (because they know that only a subset is of interest).
*/
void PlanarGraph::linkAllDirectedEdges(){
	for (map<Coordinate,Node*,CoordLT>::iterator nodeit=nodes->iterator();nodeit!=nodes->nodeMap->end();nodeit++) {
		Node *node=nodeit->second;
		((DirectedEdgeStar*)node->getEdges())->linkAllDirectedEdges();
	}
}

/**
* Returns the EdgeEnd which has edge e as its base edge
* (MD 18 Feb 2002 - this should return a pair of edges)
*
* @return the edge, if found
*    <code>null</code> if the edge was not found
*/
EdgeEnd* PlanarGraph::findEdgeEnd(Edge *e) {
	for (vector<EdgeEnd*>::iterator i=getEdgeEnds()->begin();i<getEdgeEnds()->end();i++) {
		EdgeEnd *ee=*i;
		if (ee->getEdge()==e)
			return ee;
	}
	return NULL;
}

/**
* Returns the edge whose first two coordinates are p0 and p1
*
* @return the edge, if found
*    <code>null</code> if the edge was not found
*/
Edge* PlanarGraph::findEdge(const Coordinate& p0, const Coordinate& p1) {
    unsigned int i;
	for(i=0; i<edges->size();i++) {
//        Edge *e=edges->at(i);
        Edge *e=(*edges)[i];
		const CoordinateList* eCoord=e->getCoordinates();
		if (p0==eCoord->getAt(0) && p1==eCoord->getAt(1))
			return e;
	}
	return NULL;
}

/**
* Returns the edge which starts at p0 and whose first segment is
* parallel to p1
*
* @return the edge, if found
*    <code>null</code> if the edge was not found
*/
Edge* PlanarGraph::findEdgeInSameDirection(const Coordinate& p0, const Coordinate& p1) {
	for(unsigned int i=0; i<edges->size();i++) {
		Edge *e=(*edges)[i];
//		Edge *e=edges->at(i);
		const CoordinateList* eCoord=e->getCoordinates();
		if (matchInSameDirection(p0,p1,eCoord->getAt(0),eCoord->getAt(1)))
			return e;
		if (matchInSameDirection(p0,p1,eCoord->getAt(eCoord->getSize()-1),eCoord->getAt(eCoord->getSize()-2)))
			return e;
	}
	return NULL;
}

/**
* The coordinate pairs match if they define line segments lying in the same direction.
* E.g. the segments are parallel and in the same quadrant
* (as opposed to parallel and opposite!).
*/
bool PlanarGraph::matchInSameDirection(const Coordinate& p0, const Coordinate& p1, const Coordinate& ep0, const Coordinate& ep1) {
	if (!(p0==ep0))
		return false;
	if (CGAlgorithms::computeOrientation(p0,p1,ep1)==CGAlgorithms::COLLINEAR
		&& Quadrant::quadrant(p0,p1)==Quadrant::quadrant(ep0,ep1))
			return true;
	return false;
}

string PlanarGraph::printEdges(){
	string out="Edges: ";
	for(unsigned int i=0;i<edges->size();i++) {
		out+="edge ";
		out+=i;
		out+=":\n";
		Edge *e=(*edges)[i];
//		Edge *e=edges->at(i);
		out+=e->print();
		out+=e->eiList->print();
	}
	return out;
}

NodeMap* PlanarGraph::getNodeMap(){
	return nodes;
}
}
