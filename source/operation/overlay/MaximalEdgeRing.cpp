#include "opOverlay.h"
#include "stdio.h"

MaximalEdgeRing::MaximalEdgeRing(DirectedEdge *start,GeometryFactory *geometryFactory, CGAlgorithms *cga):
EdgeRing(start,geometryFactory,cga){}

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

vector<EdgeRing*>* MaximalEdgeRing::buildMinimalRings() {
	vector<EdgeRing*> *minEdgeRings=new vector<EdgeRing*>;
	DirectedEdge *de=startDe;
	do {
		if(de->getMinEdgeRing()==NULL) {
			EdgeRing *minEr=new MinimalEdgeRing(de,geometryFactory,cga);
			minEdgeRings->push_back(minEr);
		}
		de=de->getNext();
	} while(de!=startDe);
	return minEdgeRings;
}

