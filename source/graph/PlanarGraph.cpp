#include "graph.h"

CGAlgorithms* PlanarGraph::cga=new RobustCGAlgorithms();
LineIntersector* PlanarGraph::li=new RobustLineIntersector();

 /**
 * For nodes in the vector, link the DirectedEdges at the node that are in the result.
 * This allows clients to link only a subset of nodes in the graph, for
 * efficiency (because they know that only a subset is of interest).
 */
void linkResultDirectedEdges(vector<Node*> allNodes){
	for(vector<Node*>::iterator nodeit=allNodes.begin();nodeit<allNodes.end();nodeit++) {
		Node *node=*nodeit;
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

vector<Edge*>::iterator PlanarGraph::getEdgeIterator() {
	return edges->begin();
}
vector<EdgeEnd*> *PlanarGraph::getEdgeEnds(){
	return edgeEndList;
}

bool PlanarGraph::isBoundaryNode(int geomIndex,Coordinate coord){
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
//Wouldn't work. Use iterator
//Collection PlanarGraph::getNodes() { return nodes.values(); }

Node* PlanarGraph::addNode(Node *node) {
	return nodes->addNode(node);
}

Node* PlanarGraph::addNode(Coordinate coord) {
	return nodes->addNode(coord);
}

/**
* @return the node if found; null otherwise
*/
Node* PlanarGraph::find(Coordinate coord) {
	return nodes->find(coord);
}

/**
* Add a set of edges to the graph.  For each edge two DirectedEdges
* will be created.  DirectedEdges are NOT linked by this method.
*/
void PlanarGraph::addEdges(vector<Edge*> edgesToAdd){
	// create all the nodes for the edges
	for (vector<Edge*>::iterator it=edgesToAdd.begin();it<edgesToAdd.end();it++) {
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
	for (map<Coordinate,Node*,CoordLT>::iterator nodeit=nodes->iterator();nodeit!=nodes->nodeMap.end();nodeit++) {
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
	for (map<Coordinate,Node*,CoordLT>::iterator nodeit=nodes->iterator();nodeit!=nodes->nodeMap.end();nodeit++) {
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
Edge* PlanarGraph::findEdge(Coordinate p0, Coordinate p1) {
    unsigned int i;
	for(i=0; i<edges->size();i++) {
//        Edge *e=edges->at(i);
        Edge *e=(*edges)[i];
		CoordinateList& eCoord=e->getCoordinates();
		if (p0==eCoord.getAt(0) && p1==eCoord.getAt(1))
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
Edge* PlanarGraph::findEdgeInSameDirection(Coordinate p0,Coordinate p1) {
	for(unsigned int i=0; i<edges->size();i++) {
		Edge *e=(*edges)[i];
//		Edge *e=edges->at(i);
		CoordinateList& eCoord=e->getCoordinates();
		if (matchInSameDirection(p0,p1,eCoord.getAt(0),eCoord.getAt(1)))
			return e;
		if (matchInSameDirection(p0,p1,eCoord.getAt(eCoord.getSize()-1),eCoord.getAt(eCoord.getSize()-2)))
			return e;
	}
	return NULL;
}

/**
* The coordinate pairs match if they define line segments lying in the same direction.
* E.g. the segments are parallel and in the same quadrant
* (as opposed to parallel and opposite!).
*/
bool PlanarGraph::matchInSameDirection(Coordinate p0,Coordinate p1,Coordinate ep0,Coordinate ep1) {
	if (!(p0==ep0))
		return false;
	if (cga->computeOrientation(p0,p1,ep1)==CGAlgorithms::COLLINEAR
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
