#include "../../headers/opOverlay.h"
#include "stdio.h"

MinimalEdgeRing::MinimalEdgeRing(DirectedEdge *start,GeometryFactory *geometryFactory,CGAlgorithms *cga):
	EdgeRing(start,geometryFactory,cga){
	computePoints(start);
	computeRing();
}

DirectedEdge* MinimalEdgeRing::getNext(DirectedEdge *de) {
	return de->getNextMin();
}

void MinimalEdgeRing::setEdgeRing(DirectedEdge *de,EdgeRing *er) {
	de->setMinEdgeRing(er);
}
