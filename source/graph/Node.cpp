#include "../headers/graph.h"
//#include "../headers/util.h"

Node::Node(Coordinate& newCoord, EdgeEndStar* newEdges): GraphComponent(new Label(0,Location::UNDEF)) {
	coord=newCoord;
	edges=newEdges;
}

Node::~Node(){
//	delete edges;
}

Coordinate& Node::getCoordinate() {
	return coord;
}

EdgeEndStar* Node::getEdges() {
	return edges;
}

bool Node::isIsolated() {
	return (label->getGeometryCount()==1);
}

void Node::add(EdgeEnd *e) {
	// Assert: start pt of e is equal to node point
	if (edges==NULL)
		edges=new EdgeEndStar();
	edges->insert(e);
	e->setNode(this);
}

void Node::mergeLabel(Node* n) {
	mergeLabel(n->label);
}

void Node::mergeLabel(Label* label2) {
	for (int i=0; i<2; i++) {
		int loc=computeMergedLocation(label2, i);
		int thisLoc=label->getLocation(i);
		if (thisLoc==Location::UNDEF) label->setLocation(i,loc);
	}
}

void Node::setLabel(int argIndex, int onLocation) {
	if (label==NULL) {
		label=new Label(argIndex, onLocation);
	} else
		label->setLocation(argIndex, onLocation);
}

void Node::setLabelBoundary(int argIndex) {
	int loc=Location::UNDEF;
	if (label!=NULL)
		loc=label->getLocation(argIndex);
	// flip the loc
	int newLoc;
	switch (loc){
		case Location::BOUNDARY: newLoc=Location::INTERIOR; break;
		case Location::INTERIOR: newLoc=Location::BOUNDARY; break;
		default: newLoc=Location::BOUNDARY;  break;
	}
	label->setLocation(argIndex, newLoc);
}

int Node::computeMergedLocation(Label* label2, int eltIndex){
	int loc=Location::UNDEF;
	loc=label->getLocation(eltIndex);
	if (!label2->isNull(eltIndex)) {
		int nLoc=label2->getLocation(eltIndex);
		if (loc!=Location::BOUNDARY) loc=nLoc;
	}
	return loc;
}

string Node::print(){
	string out="node "+coord.toString()+" lbl: "+label->toString();
	return out;
}
