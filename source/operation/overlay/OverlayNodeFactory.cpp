#include "opOverlay.h"
#include "stdio.h"

Node* OverlayNodeFactory::createNode(Coordinate coord) {
	return new Node(coord,new DirectedEdgeStar());
}
