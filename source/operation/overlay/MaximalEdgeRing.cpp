/*
* $Log$
* Revision 1.8  2003/10/13 17:47:49  strk
* delete statement removed
*
*/
#include "../../headers/opOverlay.h"
#include "stdio.h"

namespace geos {

MaximalEdgeRing::MaximalEdgeRing(DirectedEdge *start,GeometryFactory *geometryFactory, CGAlgorithms *cga):
EdgeRing(start,geometryFactory,cga){
	computePoints(start);
	computeRing();
}

MaximalEdgeRing::~MaximalEdgeRing(){
}

DirectedEdge* MaximalEdgeRing::getNext(DirectedEdge *de) {
	return de->getNext();
}

void MaximalEdgeRing::setEdgeRing(DirectedEdge *de,EdgeRing *er) {
	de->setEdgeRing(er);
}

/**
* For all nodes in this EdgeRing,
* link the DirectedEdges at the node to form minimalEdgeRings
*/
void MaximalEdgeRing::linkDirectedEdgesForMinimalEdgeRings() {
	DirectedEdge* de=startDe;
	do {
		Node* node=de->getNode();
		((DirectedEdgeStar*) node->getEdges())->linkMinimalDirectedEdges(this);
		de=de->getNext();
	} while (de!=startDe);
}

// This function returns a newly allocated vector of
// pointers to newly allocated MinimalEdgeRing objects.
//
vector<MinimalEdgeRing*>* MaximalEdgeRing::buildMinimalRings() {
	vector<MinimalEdgeRing*> *minEdgeRings=new vector<MinimalEdgeRing*>;
	DirectedEdge *de=startDe;
	do {
		if(de->getMinEdgeRing()==NULL) {
			MinimalEdgeRing *minEr=new MinimalEdgeRing(de,geometryFactory,cga);
			minEdgeRings->push_back(minEr);
		}
		de=de->getNext();
	} while(de!=startDe);
	return minEdgeRings;
}

}

