#include "graph.h"

NodeMap::NodeMap(NodeFactory *newNodeFact) {
	nodeFact=newNodeFact;
	nodeMap=new map<Coordinate,Node*,CoordLT>();
}

NodeMap::~NodeMap() {
	delete nodeMap;
}

Node* NodeMap::addNode(Coordinate& coord){
	Node *node=find(coord);
	if (node==NULL) {
		node=nodeFact->createNode(coord);
		(*nodeMap)[coord]=node;
	}
	return node;
}

Node* NodeMap::addNode(Node *n){
	Node *node=find(n->getCoordinate());
	if (node==NULL) {
		(*nodeMap)[n->getCoordinate()]=n;
		return n;
	}
	node->mergeLabel(n);
	return node;
}

void NodeMap::add(EdgeEnd *e) {
	Coordinate& p=e->getCoordinate();
	Node *n=addNode(p);
	n->add(e);
}

/**
 * @return the node if found; null otherwise
 */
Node* NodeMap::find(Coordinate& coord){
	map<Coordinate,Node*,CoordLT>::iterator found=nodeMap->find(coord);
	if (found==nodeMap->end())
		return NULL;
	else
		return found->second;
}

map<Coordinate,Node*,CoordLT>::iterator NodeMap::iterator() {
	return nodeMap->begin();
}

//Doesn't work yet. Use iterator.
//public Collection NodeMap::values(){
//	return nodeMap.values();
//}

vector<Node*>* NodeMap::getBoundaryNodes(int geomIndex){
	vector<Node*>* bdyNodes=new vector<Node*>();
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		if (node->getLabel()->getLocation(geomIndex)==Location::BOUNDARY)
			bdyNodes->push_back(node);
	}
	return bdyNodes;
}

string NodeMap::print(){
	string out="";
	map<Coordinate,Node*,CoordLT>::iterator	it=nodeMap->begin();
	for (;it!=nodeMap->end();it++) {
		Node *node=it->second;
		out+=node->print();
	}
	return out;
}
